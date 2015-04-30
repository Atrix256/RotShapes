mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/male_src.png Images/male_encoded.png 256 -bw Images/male_bw.png
"../CPP/DAPSE.exe" -decode Images/male_encoded.png Images/male_decoded.png 1024 1024 -debugcolors Images/male_dc.png -filtersmart -aa
