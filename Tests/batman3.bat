mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/batman_src.png Images/batman3_encoded.png 256 -bw Images/batman3_bw.png
"../CPP/DAPSE.exe" -decode Images/batman3_encoded.png Images/batman3_decoded.png 1024 1024 -debugcolors Images/batman3_dc.png -aa
