@echo off

set Config="A321", "AIDC-MRO", "AIRBUS", "AJT", "AT3", "BD100", "BELL", "Boeing", "C27J", "CAPU", "C_Series", "EC120", "F16", "IDF", "LearJet45", "NCSIST", "RE100", "S76", "S92"

set Config_count=0

for %%c in (%Config%) do (
    set /a Config_count+=1
    add.exe pmserver %%~c
)
echo.
echo Total Config: %Config_count%
