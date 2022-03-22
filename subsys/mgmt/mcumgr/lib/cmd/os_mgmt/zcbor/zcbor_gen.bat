
rem Generate C files from cddl
rem Run from /zcbor

SET c="os_mgmt.cddl"
rem create encode and decode at the same time
SET both=-c os_mgmt.cddl --default-max-qty 0 code -d -e -t
SET oc=.\source
SET oh=.\include

FOR %%x IN (echo_cmd, echo_rsp) DO zcbor %both% %%x --oc %oc%\%%x.c --oh %oh%\%%x.h

