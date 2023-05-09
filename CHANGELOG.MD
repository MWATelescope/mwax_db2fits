# Change Log

Changes in each release are listed below.

## 0.12.2 10-May-2023

* Rebuilt with fixed version of mwax_common which resolves issue with detection of VCS mode for MWAX_BUFFER and QUIT modes.

## 0.12.1 27-Apr-2023

* Updated mwax_common with MWAX_BUFFER support
* Reformatted README

## 0.12.0 5-Aug-2022

* Skipping subobs in the same observation, results in skipping, not abort. Fixes #8

## 0.11.5 1-Aug-2022

* Ensure we actually quit when we get MODE==QUIT from the next psrdada buffer

## 0.11.2 11-Jan-2022

* Changed ninputs_xgpu to ninputs as correlator can supply arbitrary number of inputs (tiles). Fixes #2

## 0.11.1 29-Nov-2021

* Fixed logging bug to show offset and new offset

## 0.11.0 29-Nov-2021

* Updated code dealing with interrupted observations

## 0.10.2 5-Nov-2021

* Added a friendly message once its started up ok

## 0.10.1 5-Nov-2021

* Removed metafits command line param as it was never used

## 0.10.0 4-Nov-2021

* Removed the generation of autocorrelation stats from mwax_db2fits. mwax_stats project will cover this now.

## 0.9.0 25-May-2021

* Version numbers of upstream components are written to the output fits files.
* New command line option: --version (-v)  now displays mwax_db2fits version number.
* Hostname, Version info, startup time, uptime, (now) time, obs_id, subobs_id are all now sent in the UDP health packets.
