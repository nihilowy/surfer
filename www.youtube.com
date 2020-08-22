//block ads for www.youtube.com, place in .local/share/surfer
document.getElementById('top-container').style.display = "none"
document.getElementById('masthead-ad').style.display = "none"
