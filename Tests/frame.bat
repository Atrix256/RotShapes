mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/frame_src.png Images/frame_encoded.png 256 -gray Images/frame_gray.png
"../CPP/DAPSE.exe" -decode Images/frame_encoded.png Images/frame_decoded.png 1024 1024 -debugcolors Images/frame_dc.png -filtersmart -aa
