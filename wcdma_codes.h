/*
 *   Copyright 2012 Erik Persson
 *
 *   This file is part of the cell-sync-usrp project.
 *
 *   cell-sync-usrp is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   cell-sync-usrp is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with cell-sync-usrp.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_WCDMA_SEQUENCES_CODES_H
#define INCLUDED_WCDMA_SEQUENCES_CODES_H

#include <complex>

typedef std::complex<float> gr_complex;

// extern gr_complex stest[];
extern const unsigned char scrambling_codes[][36][4];	// hi part is real and low part is imaginary 1 means -1 and 0 means 1
extern const gr_complex sync_codes[][512];
extern const unsigned char scrambling_code_group_table[][15];

#endif // INCLUDED_WCDMA_SEQUENCES_CODES_H
