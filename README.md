# Surfer

Simple keyboard based web browser. No tabs.

Based on webkit2gtk and gtk3.

No xlibs dependency &mdash; works on sway, weston etc. 

Look also at manual ( man surfer). 

## Adblock, videos, other info
 
 `For videos` install gstreamer packages.

 config.h - customize keys, dirs and some other settings before compile. 

 It can also launch mpv( with help of youtube-dl on supported sites) on links,
 but remember to pkill -9 mpv if it hogs your cpu, used on non supported sites.

 Also possible to use other binaries/scripts - location of dirctory of that settable in config.h
 
 Beside it can toggle JS and History. Ephemeral (priv mode) on links.
 You can also set css for sites in .surfer/tablecss.txt file. 

 Search or translate for selected text, settable in config.h 


 `For adblock:`

 Install https://github.com/jun7/wyebadblock
 Then sudo ln -s /usr/lib/wyebrowser/adblock.so /usr/lib/surfer
 
 Or block through /etc/hosts

 Or save to .surfer/adblock.json file (ie from https://easylist-downloads.adblockplus.org/easylist_min_content_blocker.json )
 

 `For JS scripts:`

 Js scripts (i.e. from  greasefork site) place in .local/share/surfer/

 name them in following way:
-i.e. for run scripts on www.youtube.com name script file identically.
-if you want script to run on all sites, name it universal.js

## Compile and install:

  
  git clone https://github.com/nihilowy/surfer.git

  cd surfer

  make &&  sudo make install

  
  Depends on webkit2gtk, gtk3 development files (install it on your distro)

## Hotkeys:

`Ctrl + click` link &mdash; open link in new window

`Ctrl + n` &mdash; new window

`Ctrl +  b` &mdash; go back

`Ctrl +  f` &mdash; go forward

`Ctrl + q` &mdash; quit

`Esc` &mdash; stop loading

`Ctrl + h` &mdash; home (bookmarks list)

`Ctrl + shift + b` &mdash; bookmark site (to remove just edit file with 
links: bookmarks in your SURFER_DIR dir)

`Ctrl + o` &mdash; toggle url bar

`Ctrl + /` &mdash; find word

`Ctrl + r` &mdash; reload page

`Ctrl + =` &mdash; zoom in

`Ctrl + -` &mdash; zoom out

`Down Arrow` &mdash; scroll down

`Up arrow` &mdash; scroll up

`Ctrl +  w` &mdash; page up 

`Ctrl +  s` &mdash; page down

`Ctrl + Shift + i` &mdash; web inspector (page source)

`Ctrl + Shift + s` &mdash; toggle user style black theme 
(/usr/share/surfer/black.css)

`Ctrl + Shift + h` &mdash; show history if enabled

`F11` &mdash; toggle fullscreen




**Edit `config.h` to change hotkeys and SURFER_DIR, SURFER_DOWNLOADS, and other settings**
