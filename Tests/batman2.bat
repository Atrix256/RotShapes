mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/batman_src.png Images/batman2_encoded.png 256 -bw Images/batman2_bw.png
"../CPP/DAPSE.exe" -decode Images/batman2_encoded.png Images/batman2_decoded.png 1024 1024 -debugcolors Images/batman2_dc.png -showradialpixels -aa
