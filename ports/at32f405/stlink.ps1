# ST-LINK + AT32 OpenOCD. Default: check connection without programming.
param([switch]$Program)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$openocdRoot = Join-Path $projectRoot 'tmp/openocd-at32'
$openocdExe = Join-Path $openocdRoot 'bin-windows_amd64/openocd.exe'
$scripts = Join-Path $openocdRoot 'scripts'
$firmware = Join-Path $PSScriptRoot '_bin/weact_at32f405/AT32F405CCT7/tinyuf2.hex'
if (!(Test-Path -LiteralPath $openocdExe)) {
    throw 'Missing AT32 OpenOCD: git clone --depth 1 https://github.com/amoxu/tool-openocd-at32.git tmp/openocd-at32'
}
$operation = 'init; targets; shutdown'
if ($Program) {
    if (!(Test-Path -LiteralPath $firmware)) { throw "Firmware not found: $firmware" }
    $firmwareTcl = $firmware.Replace('\', '/')
    $operation = "program {$firmwareTcl} verify reset exit"
}
& $openocdExe -s $scripts -f interface/stlink.cfg -c 'transport select hla_swd' `
    -f target/at32f405xx.cfg -c 'adapter speed 500' -c 'reset_config none' `
    -c 'gdb_port disabled' -c 'tcl_port disabled' -c 'telnet_port disabled' -c $operation
if ($LASTEXITCODE -ne 0) { throw "OpenOCD failed with exit code $LASTEXITCODE" }
