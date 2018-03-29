@echo off

rem GeoIP makefile.vc is horrible. We need a wrapper.

if [%1] NEQ [] (
    cd ..\..\MK_BUILD\windows\%1\geoip\geoip-api-c\libGeoIP
    nmake -f Makefile.vc
    rem The caller ./script/build/geoip should have created for
    rem us the destination directories.
    copy GeoIP.lib ..\..\..\..\..\..\MK_DIST\windows\%1\lib
    copy GeoIP.h ..\..\..\..\..\..\MK_DIST\windows\%1\include
    copy GeoIPCity.h ..\..\..\..\..\..\MK_DIST\windows\%1\include
)
