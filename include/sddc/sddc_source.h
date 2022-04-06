/* -*- c++ -*- */
/*
 * Copyright 2022 Our Stars K.K.,
 * - Noritsuna Imamura <noritsuna.imamura@ourstars.jp>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#ifndef INCLUDED_SDDC_SDDC_SOURCE_H
#define INCLUDED_SDDC_SDDC_SOURCE_H

#include <sddc/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace sddc {

    /*!
     * \brief <+description of block+>
     * \ingroup sddc
     *
     */
    class SDDC_API sddc_source : virtual public gr::sync_block
    {
     public:
      typedef std::shared_ptr<sddc_source> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of sddc::sddc_source.
       *
       * To avoid accidental use of raw pointers, sddc::sddc_source's
       * constructor is in a private implementation
       * class. sddc::sddc_source::make is the public interface for
       * creating new instances.
       */
      static sptr make(int rate, const std::string& firmware_path, const size_t device_select, const std::string& antenna);
    };

  } // namespace sddc
} // namespace gr

#endif /* INCLUDED_SDDC_SDDC_SOURCE_H */
