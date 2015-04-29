"../CPP/DAPSE.exe" -encode ../Assets/frame_src.png Images/frame_encoded.png 256 -bw Images/frame_bw.png
"../CPP/DAPSE.exe" -decode Images/frame_encoded.png Images/frame_decoded.png 1024 1024 -debugcolors Images/frame_dc.png -filtersmart -aa
