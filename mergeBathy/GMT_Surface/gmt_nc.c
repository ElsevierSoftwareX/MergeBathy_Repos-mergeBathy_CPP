/*--------------------------------------------------------------------
 *	$Id: gmt_nc.c,v 1.1 2008/08/13 16:00:29 tgray Exp $
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
 *
 *	G M T _ N C . C   R O U T I N E S
 *
 * Takes care of all grd input/output built on NCAR's NetCDF routines.
 * This version is intended to provide more general support for reading
 * NetCDF files that were not generated by GMT. At the same time, the grids
 * written by these routines are intended to be more conform COARDS conventions.
 * These routines are to eventually replace the older gmt_cdf_ routines.
 *
 * Most functions will exit with error message if an internal error is returned.
 * There functions are only called indirectly via the GMT_* grdio functions.
 *
 * Author:	Remko Scharroo
 * Date:	04-AUG-2005
 * Version:	1
 *
 * Functions include:
 *
 *	GMT_nc_read_grd_info :		Read header from file
 *	GMT_nc_read_grd :		Read data set from file
 *	GMT_nc_update_grd_info :	Update header in existing file
 *	GMT_nc_write_grd_info :		Write header to new file
 *	GMT_nc_write_grd :		Write header and data set to new file
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#define GMT_WITH_NO_PS
#define GMT_CDF_CONVENTION    "COARDS/CF-1.0"	/* grd files are COARDS-compliant */
#include "gmt.h"

EXTERN_MSC int GMT_cdf_grd_info (int ncid, struct GRD_HEADER *header, char job);
int GMT_nc_grd_info (struct GRD_HEADER *header, char job);
void GMT_nc_get_units (int ncid, int varid, char *name_units);
void GMT_nc_put_units (int ncid, int varid, char *name_units);
void GMT_nc_check_step (int n, double *x, char *varname, char *file);

int GMT_is_nc_grid (struct GRD_HEADER *header)
{	/* Returns type 18 (=nf) for new NetCDF grid,
	   type 10 (=cf) for old NetCDF grids and -1 upon error */
	int id = 13;
	return (id);
}

int GMT_nc_read_grd_info (struct GRD_HEADER *header)
{
	return (GMT_nc_grd_info (header, 'r'));
}

int GMT_nc_update_grd_info (struct GRD_HEADER *header)
{
	return (GMT_nc_grd_info (header, 'u'));
}

int GMT_nc_write_grd_info (struct GRD_HEADER *header)
{
	return (GMT_nc_grd_info (header, 'w'));
}

int GMT_nc_grd_info (struct GRD_HEADER *header, char job)
{
 /* TG Body deleted to remove dependency on NetCDF Library */
	return (GMT_NOERROR);
}

int GMT_nc_read_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* header:	grid structure header
	 * grid:	array with final grid
	 * w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex:	TRUE if array is to hold real and imaginary parts (read in real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 *
	 * Reads a subset of a grdfile and optionally pads the array with extra rows and columns
	 * header values for nx and ny are reset to reflect the dimensions of the logical array,
	 * not the physical size (i.e., the padding is not counted in nx and ny)
	 */
	 
	size_t start[5] = {0,0,0,0,0}, edge[5] = {1,1,1,1,1};
	int ndims = 0;
	GMT_LONG first_col, last_col, first_row, last_row;
	GMT_LONG i, j, width_in, width_out, height_in, i_0_out, inc = 1;
	int *k;
	size_t ij, kk;	/* To allow 64-bit addressing on 64-bit systems */
	BOOLEAN check;
	float *tmp = VNULL;

	/* Check type: is file in old NetCDF format or not at all? */

	if (GMT_grdformats[header->type][0] == 'c')
		return (GMT_cdf_read_grd (header, grid, w, e, s, n, pad, complex));
	else if (GMT_grdformats[header->type][0] != 'n')
		return (NC_ENOTNC);

	GMT_err_pass (GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);

	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];

	i_0_out = pad[0];		/* Edge offset in output */
	if (complex) {	/* Need twice as much space and load every 2nd cell */
		width_out *= 2;
		i_0_out *= 2;
		inc = 2;
	}

	/* Open the NetCDF file */

	if (!strcmp (header->name,"=")) return (GMT_GRDIO_NC_NO_PIPE);
/* TG  	GMT_err_trap (nc_open (header->name, NC_NOWRITE, &ncid)); */
	check = !GMT_is_dnan (header->nan_value);
/* TG	GMT_err_trap (nc_inq_varndims (ncid, header->z_id, &ndims)); */

	tmp = (float *) GMT_memory (VNULL, (size_t)header->nx, sizeof (float), "GMT_nc_read_grd");

	/* Load the data row by row. The data in the file is stored either "top down"
	 * (y_order < 0, the first row is the top row) or "bottom up" (y_order > 0, the first
	 * row is the bottom row). GMT will store the data in "top down" mode. */

	for (i = 0; i < ndims-2; i++) {
		start[i] = header->t_index[i];
	}
	edge[ndims-1] = header->nx;
	if (header->y_order < 0)
		ij = (size_t)pad[3] * (size_t)width_out + (size_t)i_0_out;
	else {		/* Flip around the meaning of first and last row */
		ij = ((size_t)last_row - (size_t)first_row + (size_t)pad[3]) * (size_t)width_out + (size_t)i_0_out;
		j = first_row;
		first_row = header->ny - 1 - last_row;
		last_row = header->ny - 1 - j;
	}
	header->z_min =  DBL_MAX;
	header->z_max = -DBL_MAX;

	for (j = first_row; j <= last_row; j++, ij -= ((size_t)header->y_order * (size_t)width_out)) {
		start[ndims-2] = j;
/* TG		GMT_err_trap (nc_get_vara_float (ncid, header->z_id, start, edge, tmp)); */	/* Get one row */
                 for (i = 0, kk = ij; i < width_in; i++, kk+=inc) {      /* Check for and handle NaN proxies */
			grid[kk] = tmp[k[i]];
			if (check && grid[kk] == header->nan_value) grid[kk] = GMT_f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}

	header->nx = width_in;
	header->ny = height_in;
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

/* TG	GMT_err_trap (nc_close (ncid)); */

	GMT_free ((void *)k);
	GMT_free ((void *)tmp);
	return (GMT_NOERROR);
}

int GMT_nc_write_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, NV_FLOAT64 *z)
{	/* header:	grid structure header
	 * grid:	array with final grid
	 * w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively

	 * complex:	TRUE if array is to hold real and imaginary parts (read in real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	size_t start[2] = {0,0}, edge[2] = {1,1};
	GMT_LONG i, j, inc = 1, nr_oor = 0, err, width_in, width_out, height_out;
	GMT_LONG first_col, last_col, first_row, last_row;
	size_t ij;	/* To allow 64-bit addressing on 64-bit systems */
	float *tmp_f = VNULL;
	int *tmp_i = VNULL, *k;
	double limit[2] = {FLT_MIN, FLT_MAX}, value;
	nc_type z_type;

	/* Determine the value to be assigned to missing data, if not already done so */

	switch (GMT_grdformats[header->type][1]) {
		case 'b':
			if (GMT_is_dnan (header->nan_value)) header->nan_value = CHAR_MIN;
			limit[0] = CHAR_MIN - 0.5, limit[1] = CHAR_MAX + 0.5;
			z_type = NC_BYTE; break;
		case 's':
			if (GMT_is_dnan (header->nan_value)) header->nan_value = SHRT_MIN;
			limit[0] = SHRT_MIN - 0.5, limit[1] = SHRT_MAX + 0.5;
			z_type = NC_SHORT; break;
		case 'i':
			if (GMT_is_dnan (header->nan_value)) header->nan_value = INT_MIN;
			limit[0] = INT_MIN - 0.5, limit[1] = INT_MAX + 0.5;
			z_type = NC_INT; break;
		case 'f':
			z_type = NC_FLOAT; break;
		case 'd':
			z_type = NC_DOUBLE; break;
		default:
			z_type = NC_NAT;
	}

	GMT_err_pass (GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];

	complex %= 64;	/* grd Header is always written */
	if (complex) inc = 2;

	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;
	header->nx = width_out;
	header->ny = height_out;

	/* Write grid header without closing file afterwards */

	GMT_err_trap (GMT_nc_grd_info (header, 'W'));

	/* Set start position for writing grid */

	edge[1] = width_out;
	ij = (size_t)first_col + (size_t)pad[0] + ((size_t)last_row + (size_t)pad[3]) * (size_t)width_in;
	header->z_min =  DBL_MAX;
	header->z_max = -DBL_MAX;

	/* Store z-variable. Distinguish between floats and integers */

	if (z_type == NC_FLOAT || z_type == NC_DOUBLE) {
		tmp_f = (float *) GMT_memory (VNULL, (size_t)width_in, sizeof (float), "GMT_nc_write_grd");
		for (j = 0; j < height_out; j++, ij -= (size_t)width_in) {
			start[0] = j;
			for (i = 0; i < width_out; i++) {
				value = grid[inc*(ij+k[i])];
				if (GMT_is_fnan (value))
					tmp_f[i] = (float)header->nan_value;
				else if (fabs(value) > FLT_MAX) {
					tmp_f[i] = (float)header->nan_value;
					nr_oor++;
				}
				else {
					tmp_f[i] = (float)value;
					header->z_min = MIN (header->z_min, (double)tmp_f[i]);
					header->z_max = MAX (header->z_max, (double)tmp_f[i]);
				}
				z[i + (j*width_out)] = tmp_f[i];  
 /*printf("gmt_nc_write_grd: z[%ld] = %f\n", (long) (i + (j*width_out)), z[(i + (j*width_out))]); */

			}
/* TG			GMT_err_trap (nc_put_vara_float (header->ncid, header->z_id, start, edge, tmp_f));  */
		}
		GMT_free ((void *)tmp_f);
	}

	else {
		tmp_i = (int *) GMT_memory (VNULL, (size_t)width_in, sizeof (int), "GMT_nc_write_grd");
		for (j = 0; j < height_out; j++, ij -= (size_t)width_in) {
			start[0] = j;
			for (i = 0; i < width_out; i++) {
				value = grid[inc*(ij+k[i])];
				if (GMT_is_fnan (value))
					tmp_i[i] = irint (header->nan_value);
				else if (value <= limit[0] || value >= limit[1]) {
					tmp_i[i] = irint (header->nan_value);
					nr_oor++;
				}
				else {
					tmp_i[i] = irint (value);
					header->z_min = MIN (header->z_min, (double)tmp_i[i]);
					header->z_max = MAX (header->z_max, (double)tmp_i[i]);
				}
			}
/* TG			GMT_err_trap (nc_put_vara_int (header->ncid, header->z_id, start, edge, tmp_i)); */
		}
		GMT_free ((void *)tmp_i);
	}

	if (nr_oor > 0) fprintf (stderr, "%s: Warning: %ld out-of-range grid values converted to _FillValue [%s]\n", GMT_program, nr_oor, header->name);

	GMT_free ((void *)k);

	/* Limits need to be written in actual, not internal grid, units */

	if (header->z_min <= header->z_max) {
		limit[0] = header->z_min * header->z_scale_factor + header->z_add_offset;
		limit[1] = header->z_max * header->z_scale_factor + header->z_add_offset;
	}
	else {
		fprintf (stderr, "%s: Warning: No valid values in grid [%s]\n", GMT_program, header->name);
		limit[0] = limit[1] = 0.0;
	}
/*	GMT_err_trap (nc_put_att_double (header->ncid, header->z_id, "actual_range", NC_DOUBLE, (size_t)2, limit)); */

	/* Close grid */

/*	GMT_err_trap (nc_close (header->ncid)); */

	return (GMT_NOERROR);
}

void GMT_nc_get_units (int ncid, int varid, char *name_units)
{	/* Get attributes long_name and units for given variable ID
	 * and assign variable name if attributes are not available.
	 * ncid, varid		: as in nc_get_att_text
	 * nameunit		: long_name and units in form "long_name [units]"
	 */
	return;
/* TG
	if (GMT_nc_get_att_text (ncid, varid, "long_name", name_units, (size_t)GRD_UNIT_LEN)) nc_inq_varname (ncid, varid, name_units);
	if (!GMT_nc_get_att_text (ncid, varid, "units", units, (size_t)GRD_UNIT_LEN) && units[0]) sprintf (name_units, "%s [%s]", name_units, units);
*/
}

void GMT_nc_put_units (int ncid, int varid, char *name_units)
{	/* Put attributes long_name and units for given variable ID based on
	 * string name_unit in the form "long_name [units]".
	 * ncid, varid		: as is nc_put_att_text
	 * name_units		: string in form "long_name [units]"
	 */
	int i = 0;
	char name[GRD_UNIT_LEN], units[GRD_UNIT_LEN];

	strcpy (name, name_units);
	units[0] = '\0';
	while (name[i] && name[i] != '[') i++;
	if (name[i]) {
		strcpy (units, &name[i+1]);
		name[i] = '\0';
		if (name[i-1] == ' ') name[i-1] = '\0';
	}
	i = 0;
	while (units[i] && units[i] != ']') i++;
	if (units[i]) units[i] = '\0';
/* TG
	if (name[0]) nc_put_att_text (ncid, varid, "long_name", strlen(name), name);
	if (units[0]) nc_put_att_text (ncid, varid, "units", strlen(units), units);
*/
}

void GMT_nc_check_step (int n, double *x, char *varname, char *file)
{	/* Check if all steps in range are the same (within 2%) */
	double step, step_min, step_max;
	int i;
	if (n < 2) return;
	step_min = step_max = x[1]-x[0];
	for (i = 2; i < n; i++) {
		step = x[i]-x[i-1];
		if (step < step_min) step_min = step;
		if (step > step_max) step_max = step;
	}
	if (fabs(step_min-step_max)/(fabs(step_min)+fabs(step_max)) > 0.05) {
		fprintf (stderr, "%s: WARNING: The step size of coordinate (%s) in grid %s is not constant.\n", GMT_program, varname, file);
		fprintf (stderr, "%s: WARNING: GMT will use a constant step size of %g; the original ranges from %g to %g.\n", GMT_program, (x[n-1]-x[0])/(n-1), step_min, step_max);
	}
}