mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/loki_src.png Images/loki_encoded.png 256 -bw Images/loki_bw.png
"../CPP/DAPSE.exe" -decode Images/loki_encoded.png Images/loki_decoded.png 1024 1024 -debugcolors Images/loki_dc.png -filtersmart -aa
