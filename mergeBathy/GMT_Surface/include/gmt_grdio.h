/*--------------------------------------------------------------------
 *	$Id: gmt_grdio.h,v 1.41 2008/03/24 08:58:30 guru Exp $
 *
 *	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * Include file for grd i/o
 *
 * Author:	Paul Wessel
 * Date:	21-AUG-1995
 * Revised:	06-DEC-2001
 * Version:	4
 */

#ifndef GMT_GRDIO_H
#define GMT_GRDIO_H

#include "nvtypes.h"

EXTERN_MSC int GMT_grdformats [GMT_N_GRD_FORMATS][2];

EXTERN_MSC int GMT_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, NV_FLOAT64 *z);

EXTERN_MSC int GMT_grd_data_size (int format, double *nan_value);
EXTERN_MSC int GMT_grd_prep_io (struct GRD_HEADER *header, double *w, double *e, double *s, double *n, GMT_LONG *width, GMT_LONG *height, GMT_LONG *first_col, GMT_LONG *last_col, GMT_LONG *first_row, GMT_LONG *last_row, int **index);
EXTERN_MSC int GMT_adjust_loose_wesn (double *w, double *e, double *s, double *n, struct GRD_HEADER *header);
EXTERN_MSC int GMT_grd_setregion (struct GRD_HEADER *h, double *xmin, double *xmax, double *ymin, double *ymax);
EXTERN_MSC int GMT_grd_format_decoder (const char *code);
EXTERN_MSC void GMT_grd_init (struct GRD_HEADER *header, int argc, char **argv, BOOLEAN update);
EXTERN_MSC void GMT_grd_shift (struct GRD_HEADER *header, float *grd, double shift);
EXTERN_MSC void GMT_decode_grd_h_info (char *input, struct GRD_HEADER *h);
EXTERN_MSC int GMT_grd_RI_verify (struct GRD_HEADER *h, int mode);
EXTERN_MSC int GMT_grd_get_format (char *file, struct GRD_HEADER *header, BOOLEAN magic);
EXTERN_MSC BOOLEAN GMT_grd_is_global (struct GRD_HEADER *h);

/* These are pointers to the various functions and are set in GMT_grdio_init() */

EXTERN_MSC PFI GMT_io_readinfo[GMT_N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_updateinfo[GMT_N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_writeinfo[GMT_N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_readgrd[GMT_N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_writegrd[GMT_N_GRD_FORMATS];

#include "gmt_customio.h"

struct GMT_GRID {	/* To hold a GMT float grid and its header in one container */
	struct GRD_HEADER *header;	/* Pointer to full GMT header for the grid */
	float *data;			/* Pointer to the float grid */
};

struct GMT_GRDFILE {
	int size;		/* Bytes per item */
	int n_byte;		/* Number of bytes for row */
	int row;		/* Current row */
	int fid;		/* NetCDF file number */
	size_t edge[2];		/* Dimension arrays for netCDF files */
	size_t start[2];	/* same */

	BOOLEAN check;		/* TRUE if we must replace NaNs with another representation on i/o */
	BOOLEAN auto_advance;	/* TRUE if we want to read file sequentially */
	
	double scale;		/* scale to use for i/o */
	double offset;		/* offset to use for i/o */
	
	FILE *fp;		/* File pointer for native files */
	
	void *v_row;		/* Void Row pointer for any format */
	
	struct GRD_HEADER header;	/* Full GMT header for the file */
};
	
/* Row i/o functions */

EXTERN_MSC int GMT_open_grd (char *file, struct GMT_GRDFILE *G, char mode);
EXTERN_MSC void GMT_close_grd (struct GMT_GRDFILE *G);
EXTERN_MSC int GMT_read_grd_row (struct GMT_GRDFILE *G, int row_no, float *row);
EXTERN_MSC int GMT_write_grd_row (struct GMT_GRDFILE *G, int row_no, float *row);

/* IMG read function */

EXTERN_MSC int GMT_read_img (char *imgfile, struct GRD_HEADER *h, float **grid, double w, double e, double s, double n, double scale, int mode, double lat, BOOLEAN init);

/* Grid container allocation/deallocation routines */

EXTERN_MSC struct GMT_GRID *GMT_create_grid (char *arg);
EXTERN_MSC void GMT_destroy_grid (struct GMT_GRID *G, BOOLEAN free_grid);

#endif /* GMT_GRDIO_H */
