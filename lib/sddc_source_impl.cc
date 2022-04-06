/* -*- c++ -*- */
/*
 * Copyright 2022 Our Stars K.K.,
 * - Noritsuna Imamura <noritsuna.imamura@ourstars.jp>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/io_signature.h>
#include "sddc_source_impl.h"

#include <boost/assign.hpp>
#include <boost/format.hpp>

#include <stdexcept>
#include <iostream>
#include <stdio.h>

#include <libsddc.h>

using namespace boost::assign;

#define BUF_SIZE  (1024 * 1024)
#define BUF_NUM   15
#define BUF_SKIP  1 // buffers to skip due to garbage

#define BYTES_PER_SAMPLE  8

#define DC_LOOPS 5
#define FW_PATH "~/gr-sddc/lib_so/SDDC_FX3.img"


namespace gr {
  namespace sddc {
    
    using input_type = gr_complex;
    using output_type = gr_complex;

    sddc_source::sptr
    sddc_source::make(int rate, const std::string& firmware_path, const size_t device_select, const std::string& antenna)
    {
      return gnuradio::make_block_sptr<sddc_source_impl>(
        rate, firmware_path, device_select, antenna);
    }

    /*
     * The private constructor
     */
    sddc_source_impl::sddc_source_impl(int rate, const std::string& firmware_path, const size_t device_select, const std::string& antenna)
      : gr::sync_block("sddc_source",
              gr::io_signature::make(0 /* min inputs */, 0 /* max inputs */, sizeof(input_type)),
              gr::io_signature::make(1 /* min outputs */, 1 /*max outputs */, sizeof(output_type)))
    {
      int dev_index = device_select;
      if( dev_index < 0) {
        dev_index = 0;
      }

      _buf_num = _buf_head = _buf_used = _buf_offset = 0;
      _samp_avail = BUF_SIZE / BYTES_PER_SAMPLE;
      _buf_num = BUF_NUM;

      if ( dev_index >= sddc_get_device_count() )
        throw std::runtime_error("Wrong sddc device index given.");

      std::cerr << "Using device #" << dev_index << ": "
                << std::endl;

      _dev = NULL;
      if( firmware_path.empty() ) {
        _dev = sddc_open(dev_index, FW_PATH);
        _firmware_path = FW_PATH;
      } else {
        _dev = sddc_open(dev_index, firmware_path.c_str());
        _firmware_path = firmware_path;
      }
      if (!_dev)
        throw std::runtime_error("Failed to open sddc device.");


      sddc_stop_streaming(_dev);

      _buf = (unsigned short **) malloc(_buf_num * sizeof(unsigned short *));
      _buf_lens = (unsigned int *) malloc(_buf_num * sizeof(unsigned int));

      if (_buf && _buf_lens) {
        for(unsigned int i = 0; i < _buf_num; ++i) {
          _buf[i] = (unsigned short *) malloc(BUF_SIZE);
        }
      }
      if( set_sample_rate(rate) <= 0) {
        ~sddc_source_impl();
        throw std::runtime_error("Failed to set sampling rate to sddc device.");
      }
      if( sddc_set_async_params(_dev, 0, 0, sddc_source_impl::_sddc_callback, this) < 0) {
        ~sddc_source_impl();
        throw std::runtime_error("Failed to set async params to sddc device.");
      }
      if( sddc_start_streaming(_dev) < 0) {
        ~sddc_source_impl();
        throw std::runtime_error("Failed to start streaming from sddc device.");
      }
    }

    /*
     * Our virtual destructor.
     */
    sddc_source_impl::~sddc_source_impl()
    {
      if (_dev) {
        _running = false;
        sddc_stop_streaming(_dev);
        sddc_close(_dev);
        _dev = NULL;
      }

      if (_buf) {
        for(unsigned int i = 0; i < _buf_num; ++i) {
          free(_buf[i]);
        }

        free(_buf);
        _buf = NULL;
        free(_buf_lens);
        _buf_lens = NULL;
      }
    }


    void
    sddc_source_impl::_sddc_callback(uint32_t len, const float *buf, void *ctx)
    {
      sddc_source_impl *obj = (sddc_source_impl *)ctx;
      obj->sddc_callback((unsigned char *)buf, len * 4);
    }

    void
    sddc_source_impl::sddc_callback(unsigned char *buf, uint32_t len)
    {
      if (_skipped < BUF_SKIP) {
        _skipped++;
        return;
      }

      {
        boost::mutex::scoped_lock lock( _buf_mutex );

        if (len > BUF_SIZE)
        {
          std::cerr<<"len="<<len<<" BUF_SIZE="<<BUF_SIZE<<std::endl;
          throw std::runtime_error("Buffer too small.");
        }

        int buf_tail = (_buf_head + _buf_used) % _buf_num;
        memcpy(_buf[buf_tail], buf, len);
        _buf_lens[buf_tail] = len;

        if (_buf_used == _buf_num) {
          std::cerr << "O" << std::flush;
          _buf_head = (_buf_head + 1) % _buf_num;
        } else {
          _buf_used++;
        }
      }

      _buf_cond.notify_one();
    }

    void 
    sddc_source_impl::rearm_dcr()
    {
      _dc_size = sddc_get_sample_rate(_dev);
      _dc_loops = 0;
      _dc_count = 0;
      _dc_accum = gr_complex(0.0);
    }


    int 
    sddc_source_impl::work( int noutput_items,
                            gr_vector_const_void_star &input_items,
                            gr_vector_void_star &output_items )
    {
      gr_complex *out = (gr_complex *)output_items[0];
      int processed = 0;

      {
        boost::mutex::scoped_lock lock( _buf_mutex );

        while (_buf_used < 3 && _running) { // collect at least 3 buffers
          _buf_cond.wait( lock );
        }
      }

      if (!_running)
        return WORK_DONE;

      gr_complex *buf = (gr_complex *)_buf[_buf_head] + _buf_offset;

      if (noutput_items <= _samp_avail) {
        for (int i = 0; i < noutput_items; i++) {
          *out++ = *(buf + i) - _dc_offset;
        }

        _buf_offset += noutput_items;
        _samp_avail -= noutput_items;
        processed += noutput_items;
      } else {
        for (int i = 0; i < _samp_avail; i++) {
          *out++ = *(buf + i) - _dc_offset;
        }

        {
          boost::mutex::scoped_lock lock( _buf_mutex );

          _buf_head = (_buf_head + 1) % _buf_num;
          _buf_used--;
        }

        buf = (gr_complex *)_buf[_buf_head];

        int remaining = noutput_items - _samp_avail;
        processed += _samp_avail;
        if(remaining > (int)_buf_lens[_buf_head] / BYTES_PER_SAMPLE) {
          remaining = _buf_lens[_buf_head] / BYTES_PER_SAMPLE;
        }

        for (int i = 0; i < remaining; i++) {
          *out++ = *(buf + i) - _dc_offset;
        }

        _buf_offset = remaining;
        _samp_avail = (_buf_lens[_buf_head] / BYTES_PER_SAMPLE) - remaining;
        processed += remaining;
      }
      if(_dc_loops < DC_LOOPS) {
        out = (gr_complex *)output_items[0];
        gr_complex loffset = gr_complex(0.0);
        for(int k = 0; k < processed; k++, out++) {
          _dc_accum += *out - loffset;
          _dc_count++;
          if(_dc_count == _dc_size) {
            _dc_offset += _dc_accum / float(_dc_size);
            loffset += _dc_accum / float(_dc_size);
            _dc_accum = gr_complex(0.0);
            _dc_count = 0;
            _dc_loops++;
            if(_dc_loops == DC_LOOPS) {
                break;
            }
          }
        }
      }
      return processed;
    }



    std::vector<int>
    sddc_source_impl::get_devices()
    {
      std::vector<int> devices;

      for (int i = 0; i < sddc_get_device_count(); i++) {
        devices.push_back( i );
      }

      return devices;
    }

    size_t 
    sddc_source_impl::get_num_channels()
    {
      return 1;
    }

    int 
    sddc_source_impl::set_sample_rate(int rate)
    {
      if (_dev) {
        sddc_set_sample_rate( _dev, (uint32_t)rate );
        rearm_dcr();
      }

      return get_sample_rate();
    }

    int 
    sddc_source_impl::get_sample_rate()
    {
      if (_dev)
        return (int)sddc_get_sample_rate( _dev );

      return 0;
    }



    std::string 
    sddc_source_impl::set_antenna( const std::string & antenna, size_t chan )
    {
      return get_antenna( chan );
    }

    std::string 
    sddc_source_impl::get_antenna( size_t chan )
    {
      return "RX1";
    }


  } /* namespace sddc */
} /* namespace gr */
