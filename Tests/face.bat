mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/face_src.bmp Images/face_encoded.png 256 -gray Images/face_gray.png
"../CPP/DAPSE.exe" -decode Images/face_encoded.png Images/face_decoded.png 1024 1024 -debugcolors Images/face_dc.png -filtersmart -aa
