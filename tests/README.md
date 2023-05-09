# Tests for mwax_db2fits

## About

mwax_db2fits is a C program that utilises PSRDADA ringbuffers for input. In order to create an effective test suite, we must generate test headers and test data, load into the ring buffer, then run mwax_db2fits, which will either fail (hopefully on purpose) or succeed creating a FITS file. The log from mwax_db2fits and/or the output FITS file from each test will be examined to determine if each test scenario passes. The tests are all run via pytest.

## Running tests

Simply: $ ./run_tests.sh

## Tests

### Test 01: Normal Observation

See [test01/README.md](test01/README.md) for details.

### Test 02: Subobs jumps in time by more than 8 seconds

See [test02/README.md](test02/README.md) for details.
