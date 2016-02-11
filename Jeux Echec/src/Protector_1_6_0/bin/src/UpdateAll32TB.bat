Release\protector.exe -t pgn\wac.pgn  -e c:\tablebases
del wac.log
ren protector.log wac.log

Release\protector.exe -t pgn\easy.pgn  -e c:\tablebases
del easy.log
ren protector.log easy.log

Release\protector.exe -t pgn\hard.pgn -e c:\tablebases
del hard.log
ren protector.log hard.log

Release\protector.exe -t pgn\problems.pgn -e c:\tablebases
del problems.log
ren protector.log problems.log

Release\protector.exe -t pgn\ecm.pgn -e c:\tablebases
del ecm.log
ren protector.log ecm.log

Release\protector.exe -t pgn\wcs.pgn -e c:\tablebases
del wcs.log
ren protector.log wcs.log
