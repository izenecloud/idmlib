/* cdf/gsl_cdf.h
 *
 * Copyright (C) 2002 Jason H. Stover.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */

/* Author:  J. Stover */

#ifndef __GSL_CDF_H__
#define __GSL_CDF_H__

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS          /* empty */
# define __END_DECLS            /* empty */
#endif

__BEGIN_DECLS

double gsl_cdf_ugaussian_P (const double x);
double gsl_cdf_ugaussian_Q (const double x);

double gsl_cdf_gaussian_P (const double x, const double sigma);
double gsl_cdf_gaussian_Q (const double x, const double sigma);

double gsl_ran_gaussian_pdf (const double x, const double sigma);

__END_DECLS

#endif /* __GSL_CDF_H__ */
