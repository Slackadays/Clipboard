# How I Made the Clipboard Project

I made the Clipboard Project (which I'll call CB here) by first starting with something really small and simple but functional, then adding features upon features until I got to where it is right now.

The Ugly Details:

1. First, I started by thinking about how I wanted CB to work in the first place. Because if I don't know what I want the final product to be, then I can't work towards anything. One huge, and HUGE, assumption I made was that every single time you use CB, it should only
 perform a single, atomic action. There shouldn't be any funny business where some invocations do multiple things at once. Therefore, this helped me structure CB by assuming that every time it runs, it'll perform only one distinct action, and I can build everything else around that assumption.

2. Next, I had to decide what language to write CB in. I already knew C++ from Hajime, one of my earlier projects. And I've been learning it since I was 8! Plus, it's super fast and filled with tons of fun features starting with the C++17 standard. So, C++ it was!

3. After deciding on the modus operandi and programming language, I made a super basic version of CB. I actually wrote this initial version in Bash. That was due to me wanting to see if such a task could be done in a "simpler" language before moving to the big guns of C++. Unfortunately, it didn't
 turn out so well. The Bash version was filled with redundant code and was hard to work on. Due to me not wanting to suffer any further, I rewrote the Bash version in C++.

4. I started adding more and more features after sharing my CB project online and then receiving valuable feedback. I also researched other, similar projects like CopyQ, Maccy, Xclip, and wl-clipboard, and incorporating their features in a way that would suit the CB philosophy best.

5. From there, it's been smooth sailing and in fact a little boring because I additionally incorporated automated code formatting, a contribution from another fellow developer, integration tests, and a coherent design language. The tests have kept bugs and regressions to a minimum. The code formatting keeps everything looking tidy. And, the design language makes CB one
 of the most beautiful tools to use on the command line.
