mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/batman_src.png Images/batman_encoded.png 256 -gray Images/batman_gray.png
"../CPP/DAPSE.exe" -decode Images/batman_encoded.png Images/batman_decoded.png 1024 1024 -debugcolors Images/batman_dc.png -filtersmart -aa
