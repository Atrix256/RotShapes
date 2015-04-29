mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/test_src.png Images/test_encoded.png 256 -gray Images/test_gray.png
"../CPP/DAPSE.exe" -decode Images/test_encoded.png Images/test_decoded.png 1024 1024 -debugcolors Images/test_dc.png -filtersmart -aa
