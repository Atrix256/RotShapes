mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/malefemale_src.png Images/malefemale_encoded.png 256 -bw Images/malefemale_bw.png
"../CPP/DAPSE.exe" -decode Images/malefemale_encoded.png Images/malefemale_decoded.png 1024 1024 -debugcolors Images/malefemale_dc.png -filtersmart -aa
