mkdir Images
"../CPP/DAPSE.exe" -encode ../Assets/batman_src.png Images/batman4_encoded.png 256 -bw Images/batman4_bw.png
"../CPP/DAPSE.exe" -decode Images/batman4_encoded.png Images/batman4_decoded.png 1024 1024 -debugcolors Images/batman4_dc.png -filterbilinear -aa
