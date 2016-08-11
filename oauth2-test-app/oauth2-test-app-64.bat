@echo off

call "%~dp0\o4w-build-env-64.bat"

start "OAuth2 Test app" /B Z:\DepsSrc\qgis-oauth-method-plugin-install\bin\oauth2testapp.exe %*
