
rem Generate C files from cddl
rem Run from /fs_mgmt/zcbor

SET c="fs_mgmt.cddl"
rem create encode and decode at the same time
SET both=-c fs_mgmt.cddl --default-max-qty 0 code -d -e -t
SET oc=.\source
SET oh=.\include

rem At this time everything is command/response

FOR %%x IN (file_download_cmd, file_download_rsp, file_upload_cmd, file_upload_rsp) DO zcbor %both% %%x --oc %oc%\%%x.c --oh %oh%\%%x.h

