"../CPP/DAPSE.exe" -encode ../Assets/batman_src.png ../Assets/test/batman_encoded.png 256 -bw ../Assets/test/batman_bw.png
"../CPP/DAPSE.exe" -decode ../Assets/test/batman_encoded.png ../Assets/test/batman_decoded.png 1024 1024 -debugcolors ../Assets/test/batman_dc.png -filtersmart -aa
