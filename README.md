# oc-err

A header-only library contains the following error handling tools:

* `oc::optinal` and `oc::expected` types with same API as the standard library.

* Exception reporting/throwing macro `OCERR_REQUIRE(<condition>, <exeption_type>, <desc>)`
  This macro throws exception `<exception_type>` with description `<desc>` if `<condition>` is false.
  e.g.:
  
  ```cpp
  OCERR_REQUIRE(0 == 1, std::runtime_error, "comparison error")
  
  // throws std::runtime_error exeption with the following message:
  // exception (at line 10, foo@bar.cpp), assertion 0 == 1 failed: comparison error
  ```
