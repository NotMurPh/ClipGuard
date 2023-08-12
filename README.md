# ClipGuard

Prevents losing the clipboard upon closing an app in Xorg 💂🏻

## The Problem 🤔

In xorg-xserver by default without using a clipboard manager there is this problem that lets say for example you just copied somthing from chrome and then you close chrome because you no longer need it and then you try to paste the thing that you just copied but pufff 💨 nothing gets pasted !!!

And the reason for that is that the xorg-xserver doesn't actually save any of your clipboard content it just sends it straight from one program to another that means that your clipboard content from chrome is stored in chrome and not in xorg-xserver so when you close chrome, chrome no longer can send the clipboard content when requested 🥲

Well honestly this is really annoying 😠 this leaves us with one solution only and that is to use a clipboard manager since they will usually become the owner of the clipboard and everybody asks them for the clipboard content so you're free to close chrome but not the clipboard manager as the content is stored by your clibpard manager instead

Buttt 🍑 the clipboard managers usually come with so meny other things like keeping the history of the contents , an ui , copy pasting files and etc.. that means that they're usally bloatware for it to just solve this simple problem and as an Arch user btw i dont like bloatwere on my system that's why i present to you drummroll plssss 🥁🥁🥁🥁 ... The one and only the Clipboard Guardian 💂🏻 or just ClipGuard for short 🫠

## Features 📝

- Supports multiple selections ( Read the [Usage](https://github.com/NotMurPh/ClipGuard#usage-) section for more information about selections )
- Supports every content type ( text , image , etc.. )
- Supports large files ( INCR )
- It's light weight
- It's fast
- It has a easily readable code
- It Doesn't sleep on the job
- It Comes pre packaged with a charger unlike other clipboard managers which i dont name, jk jk 😂

## Installation 💾

For installation you have two options, Either clone and compile or download the binary

### Compilation 🏗️

To compile the project make sure you have the libraries `libx11` and `lib32-libx11` installed then execute the following commands:

```bash

git clone https://github.com/NotMurPh/ClipGuard.git && cd ClipGuard
g++ ClipGuard.cpp Selection.cpp  -o ClipGuard -l X11

```

### Download 📥

Just download the latest relase from this page

## Usage 🧑‍💻

To summon the guardian you must do the summoning ritual 🌀

Before you go sorceress 🧙‍♂️ firstly you need to know what are selecitons, Basically in xorg-xserver clipboard is called selection and there are different types of clipboards or selections for example there is one called `PRIMARY` which is responsable for sharing content when you select somthing with your mouse and the paste it using the middle mouse click ( Pressing the scroll button ) and to make matters more confusing there is a selection called `CLIPBOARD` which is responsable for sharing content when you use `CTRL + C` and `CTRL + V` or the normal copy paste as you'd say also you can use whatever selection you want for example selection `PENGUIN` which isn't used by any program really

Now that you have the required knowledge grab your wand 🪄 and do the ritual carefully as i say

Run the program at dawn 🌅 with your desired selections as arguments like so:

```Bash
# To run the program with the default selections PRIMARY and CLIPBOARD:

./ClipGuard

# To run the program only with selection CLIPBOARD: 

./Clipboard CLIPBOARD

# To run the program with selections CLIPBOARD and PRIMARY and PENGUIN:

./Clipboard CLIPBOARD PRIMARY PENGUIN

# To make the program run in the background

./Clipboard &

# To make the program run silently

./Clipboard > /dev/null

# Combine the two

./Clipboard > /dev/null &

```

Now that you know what's needed to be done go, go and summon the gurdian 💂🏻 May he always protects you!

## Thanks for visiting 🙃

Leave a like if you like 😉

Enjoy!
