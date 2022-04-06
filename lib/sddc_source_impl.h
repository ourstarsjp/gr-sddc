/* -*- c++ -*- */
/*
 * Copyright 2022 Our Stars K.K.,
 * - Noritsuna Imamura <noritsuna.imamura@ourstars.jp>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_SDDC_SDDC_SOURCE_IMPL_H
#define INCLUDED_SDDC_SDDC_SOURCE_IMPL_H

#include <sddc/sddc_source.h>

#include <gnuradio/thread/thread.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

struct sddc;

namespace gr {
  namespace sddc {

    class sddc_source_impl : public sddc_source
    {
     public:
      sddc_source_impl( int rate, const std::string& firmware_path, const size_t device_select, const std::string& antenna );
      ~sddc_source_impl();

      // Where all the action really happens
      int work(
              int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items
      );

      int set_sample_rate( int rate );
      int get_sample_rate( void );
      
      std::string set_antenna( const std::string &antenna, size_t chan = 0 );
      std::string get_antenna( size_t chan = 0 );

      std::vector<int> get_devices();
      size_t get_num_channels();


     private:
      static void _sddc_callback( uint32_t len, const float *buf, void *ctx );
      void sddc_callback( unsigned char *buf, uint32_t len );
      void rearm_dcr();

      struct sddc *_dev;
      unsigned short **_buf;
      unsigned int *_buf_lens;
      unsigned int _buf_num;
      unsigned int _buf_head;
      unsigned int _buf_used;
      boost::mutex _buf_mutex;
      boost::condition_variable _buf_cond;
      bool _running;

      unsigned int _buf_offset;
      int _samp_avail;

      unsigned int _skipped;
      gr_complex _dc_offset;
      gr_complex _dc_accum;
      int _dc_loops;
      int _dc_count;
      int _dc_size;

      std::string _firmware_path;

    };

  } // namespace sddc
} // namespace gr

#endif /* INCLUDED_SDDC_SDDC_SOURCE_IMPL_H */
