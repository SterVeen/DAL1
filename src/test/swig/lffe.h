/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2007 by Joseph Masters                                  *
 *   jmasters@science.uva.nl                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "dal.h"

/*
class LFFE {
private:
	int ant1;
	int ant2;
	dalcomplex data[4][128];
	double time;
	double u;
	double v;
	double w;
public:
	int get_ant1();
	int get_ant2();
	dalcomplex get_data(int polarization, int channel);
	double get_time();
	double get_u(int n);
	double get_v();
	double get_w();
};
*/

typedef struct LFFE_ROW {
	int ant1;
	int ant2;
	dalcomplex data[4][128];
	double time;
	double u;
	double v;
	double w;
} LFFE_ROW;

/*!
  \class LFFE
  \ingroup DAL
  \author Joseph Masters
 */
class LFFE {

  LFFE_ROW row[500];
  
 public:

  //! Get data for antenna 1
  int get_ant1(int n);
  //! Get data for antenna 2
  int get_ant2(int n);
  //! Get complex data for given \e polarization and \e channel
  dalcomplex get_data (int n, int polarization, int channel);
  double get_time(int n);
  double get_u(int n);
  double get_v(int n);
  double get_w(int n);
};
