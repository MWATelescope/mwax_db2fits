/*
 * fitswriter.h
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
 #ifndef XC_FITSWRITER_H_
 #define XC_FITSWRITER_H_

 int create_fits(fitsfile **fptr, const char* filename);
 int close_fits(fitsfile *fptr);
 int open_fits(fitsfile **fptr, const char* filename);

 #endif /* XC_FITSWRITER_H_ */
