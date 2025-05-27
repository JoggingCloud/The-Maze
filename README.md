![image](https://github.com/user-attachments/assets/067b24f9-5785-4270-b05d-8e2319547eec)
About
---------------------------------------------------------------------------------------------
This is a foundational AI-focused project I built in my custom C++ engine, designed to test and showcase pathfinding and basic agent behavior in a grid-based environment. There’s no combat legitimate combat system and this isn’t about flashy mechanics. Instead, I focused purely on getting AI agents (represented as simple cylinders) to move intelligently through the world. Each AI uses a custom A* pathfinding implementation that I wrote from scratch, complete with a custom heap-based open list, intercardinal direction-based movement, and customizable callbacks for grid logic like solid tiles or diagonal movement.
The system is fully multithreaded through my job system, allowing AI agents to request and receive paths asynchronously without blocking the main thread. They operate in simple state machines (like PATROL and CHASE), where agents can detect the player and adjust their goals accordingly. I included debug visualization for AI paths, goals, and sensing to clearly show how each decision is made in real time. This project isn’t about polish, it’s about core functionality as this is where I tested and refined my engine's AI, pathfinding, and job system architecture 

How To Use
---------------------------------------------------------------------------------------------
The game also provides you with instructions on how to play the game and what you need to do to beat the game. 
The Keyboard Controls:
1.	W and S key to move in the forward and backward direction
2.	A and D key to move left or right
3.	Hold shift to sprint
4.	F key to toggle between perspective and free-fly camera mode
	
Perspective Mode:
	
 	* No mouse movement 
	* WASD to move the player actor 

Free-Fly Mode:
	* Use mouse to look in any direction which dictates the direction you will move forward in 

How To Run Application
---------------------------------------------------------------------------------------------
Before anything you must have the Navisyn-Engine already downloaded as this won't run without it. It must also be in the same file path as this project.

Method A:
1.	Extract the zip folder to your desired location
2.	Open the following path --> …\DFS2MarkovChains\Run
3.	Double-click DFS2MarkovChains_Release_x64.exe to start the program
   
Method B:
1.	Extract the zip folder to you desired location.
2.	Open the following path --> …\DFS2MarkovChains
3.	Open the DFS2MarkovChains.sln using Visual Studio 2022 and make sure the solution config and platforms are "Release" and "x64".
4.	Press F6 key to build solution or go to Build --> Build Solution option using the Menu bar.
5.	Press Ctrl + F5 key to start the program without debugging or go to Debug --> Start without Debugging option using the Menu bar.
  NOTE:	
	  * Visual Studio 2022 must be installed on the system.
	  * In step 5 mentioned above, if you want you can execute the program with the debugger too.
