@echo off
node "C:/ti/sysconfig_1.28.0/dist/cli.js" --product "mspm0-sdk/mspm0_sdk_2_11_00_07/.metadata/product.json" --device MSPM0G3507 --package "LQFP-64(PM)" --compiler gcc --output "Core/Src/" "sysconfig/MSPM0G3507.syscfg"
echo.
echo ======== SysConfig done ========
pause
