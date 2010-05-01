@REM MPEG CLASS D
FOR %%d in ( BasketballPass ) do FOR %%f in ( 23.54   27.6    31.15   33.15   36.12  	) do  CALL Encoding_IPPP.bat  %%d %%f  64 4 500 0
FOR %%d in ( BQSquare       ) do FOR %%f in ( 22.182  25.455  28.252  29.833  32.13  	) do  CALL Encoding_HierP.bat %%d %%f  64 4 600 1
FOR %%d in ( BlowingBubbles ) do FOR %%f in ( 22.712  26.196  29.34   31.054  33.678 	) do  CALL Encoding_HierP.bat %%d %%f  64 4 500 0
FOR %%d in ( RaceHorses     ) do FOR %%f in ( 21.8    25.7    29.07   30.94   33.65  	) do  CALL Encoding_IPPP.bat  %%d %%f  64 4 300 0

@REM MPEG CLASS C
FOR %%d in ( BasketballDrill ) do FOR %%f in ( 25.85   29.37   32.5    35.57   37.7   	) do  CALL Encoding_HierP.bat %%d %%f 128 5 500 0
FOR %%d in ( BQMall          ) do FOR %%f in ( 27      30.4333 33.6733 36.6333 38.74  	) do  CALL Encoding_IPPP.bat  %%d %%f 128 5 600 0
FOR %%d in ( PartyScene      ) do FOR %%f in ( 29.752  32.888  35.68   37.976  39.672 	) do  CALL Encoding_HierP.bat %%d %%f 128 5 500 0
FOR %%d in ( RaceHorsesC     ) do FOR %%f in ( 28.6    31.599  34.2269 36.72   38.477 	) do  CALL Encoding_IPPP.bat  %%d %%f 128 5 300 0

@REM MPEG CLASS E
FOR %%d in ( Vidyo1          ) do FOR %%f in ( 23.1    26.0914 29.3215 31.3838 34.5362 	) do  CALL Encoding_HierP.bat %%d %%f 128 5 600 0
FOR %%d in ( Vidyo3          ) do FOR %%f in ( 24.2    27.3184 30.53   32.7    35.95   	) do  CALL Encoding_HierP.bat %%d %%f 128 5 600 0
FOR %%d in ( Vidyo4          ) do FOR %%f in ( 23.65   26.3337 28.9656 30.83   33.8209 	) do  CALL Encoding_HierP.bat %%d %%f 128 5 600 0

@REM MPEG CLASS B
FOR %%d in ( Kimono          ) do FOR %%f in ( 21.68   24.2962 27.4054 30.4795 33.6161 	) do  CALL Encoding_IPPP.bat  %%d %%f 128 5 240 0
FOR %%d in ( ParkScene       ) do FOR %%f in ( 23.2254 25.5398 28.3065 30.9506 33.7929 	) do  CALL Encoding_HierP.bat %%d %%f 128 5 240 0
FOR %%d in ( Cactus          ) do FOR %%f in ( 24.5    26.1455 28.539  31.1612 33.9839 	) do  CALL Encoding_HierP.bat %%d %%f 128 5 500 0
FOR %%d in ( BasketballDrive ) do FOR %%f in ( 25.5321 27.4641 30.1776 32.9891 35.829  	) do  CALL Encoding_IPPP.bat  %%d %%f 128 5 500 0
FOR %%d in ( BQTerrace       ) do FOR %%f in ( 26.2    27.27   28.54   30.207  31.9669 	) do  CALL Encoding_HierP.bat %%d %%f 128 5 600 1
