@echo off

rem GeoIP makefile.vc is horrible. We need a wrapper.

if [%1] NEQ [] (
    cd ..\..\MK_BUILD\windows\%1\geoip\geoip-api-c\libGeoIP
    nmake -f Makefile.vc
    mkdir ..\..\..\..\..\..\MK_DIST\windows\%1\geoip\lib
    copy GeoIP.lib ..\..\..\..\..\..\MK_DIST\windows\%1\geoip\lib
    mkdir ..\..\..\..\..\..\MK_DIST\windows\%1\geoip\include
    copy GeoIP.h ..\..\..\..\..\..\MK_DIST\windows\%1\geoip\include
    copy GeoIPCity.h ..\..\..\..\..\..\MK_DIST\windows\%1\geoip\include
)
