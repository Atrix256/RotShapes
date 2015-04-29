"../CPP/DAPSE.exe" -encode ../Assets/batman_src.png Images/anim1.png 256
"../CPP/DAPSE.exe" -encode ../Assets/face_src.bmp Images/anim2.png 256
"../CPP/DAPSE.exe" -encode ../Assets/female_src.png Images/anim3.png 256
"../CPP/DAPSE.exe" -encode ../Assets/frame_src.png Images/anim4.png 256
"../CPP/DAPSE.exe" -encode ../Assets/male_src.png Images/anim5.png 256
"../CPP/DAPSE.exe" -encode ../Assets/malefemale_src.png Images/anim6.png 256
"../CPP/DAPSE.exe" -encode ../Assets/test_src.png Images/anim7.png 256

"../CPP/DAPSE.exe" -combine Images/anim1.png Images/anim2.png Images/anim.png 
"../CPP/DAPSE.exe" -combine Images/anim.png Images/anim3.png Images/anim.png 
"../CPP/DAPSE.exe" -combine Images/anim.png Images/anim4.png Images/anim.png 
"../CPP/DAPSE.exe" -combine Images/anim.png Images/anim5.png Images/anim.png 
"../CPP/DAPSE.exe" -combine Images/anim.png Images/anim6.png Images/anim.png 
"../CPP/DAPSE.exe" -combine Images/anim.png Images/anim7.png Images/anim.png 
"../CPP/DAPSE.exe" -combine Images/anim.png Images/anim1.png Images/anim.png 

"../CPP/DAPSE.exe" -animate Images/animated.gif 30 1 -decode Images/anim.png Images/animo%%i.png 1024 1024 -debugcolors Images/animodc%%i.png -filtersmart -aa