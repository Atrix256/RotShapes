mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/female_src.png Images/female_encoded.png 256 -gray Images/female_gray.png
"../CPP/DAPSE.exe" -decode Images/female_encoded.png Images/female_decoded.png 1024 1024 -debugcolors Images/female_dc.png -filtersmart -aa
