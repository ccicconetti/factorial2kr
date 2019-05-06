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
   filename: object.h
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           definition of the object superclass
*/

#ifndef __MEASURE_OBJECT_H
#define __MEASURE_OBJECT_H

#ifdef DEBUG
#include <iostream>
#endif // DEBUG

#include <string>

//! Object superclass. All other classes should inherit from this class.
class Object
{
  //! Name of the derived class.
  std::string className;
  //! Unique numerical sequential identifier.
  unsigned int id;

 public:
  //! Default construction only allowed.
  Object(std::string name)
      : className(name) {
    static unsigned int newId = 0;
    id                        = ++newId;
#ifdef DEBUG
    std::cerr << "+ " << className << " (" << id << ")\n";
#endif // DEBUG
  }
  //! Access function that returns the object unique identifier.
  unsigned int getId() const {
    return id;
  }
  //! Access function that returns the class name of this object.
  std::string getName() const {
    return className;
  }
  //! Virtual destructor, which makes this a pure virtual class.
  virtual ~Object() {
#ifdef DEBUG
    std::cerr << "- " << className << " (" << id << ")\n";
#endif // DEBUG
  }
};

#endif // __MEASURE_OBJECT_H
