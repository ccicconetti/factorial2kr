/*
 *  Copyright (C) 2006 Dip. Ing. dell'Informazione, University of Pisa, Italy
 *  http://info.iet.unipi.it/~cng/ns2measure/ns2measure.html
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA, USA
 */

/**
   project: measure
   filename: config.h
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           configuration parameters and utility functions
*/

#ifndef __MEASURE_CONFIG_H
#define __MEASURE_CONFIG_H

//! Sample type defined as a double.
typedef double sample_t;

//! Metric type: averaged or distribution.
enum MetricType { METRIC_AVG, METRIC_DIST, METRIC_NONE };

//! Maximum metric name size (including trailing '\0')
#define MAX_METRIC_NAME 1024

//! Maximum line length
#define MAX_LINE 1024

//! Reallocates vector sizes in chunk of this size.
#define VECTOR_CHUNK_SIZE 1024

//! Buffer size when copying a file
#define COPY_BUFFER_SIZE 65536

#endif // __MEASURE_CONFIG_H
