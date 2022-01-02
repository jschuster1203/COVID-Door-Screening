# COVID-Door-Screening
This is a COVID-19 symptom door screening system. This systems purpose is to determine whether a person trying to enter a room is COVID negative by asking the user various questions and taking a temperature reading. The system also monitors the occupancy of a room to make sure that there are not too many people in one room.

Video 1 Link: https://drive.google.com/file/d/10RBgkNKbW8vOPP0VAnyErfPoyMYO6g5A/view?usp=sharing

Video 1 Description: This video is a normal walkthrough of the whole screening process with starting from 3 feet away, going up to the door, and the user answering all the questions “no” saying they do not have any COVID-19 symptoms. When compiling and uploading the program to the board, we set the maximum capacity to 5 people with 2 people already being in the room. When the person completes the screening, this will up the current occupancy to 3 which will change the LED from green(<=40%) to yellow(between 41 and 79%). At the very end, the OLED screen just says “No symptoms reported, please enter”. 

Video 2 Link: https://drive.google.com/file/d/10U68Vzq-1mPPlWkni-VmwltNSzo550zX/view?usp=sharing

Video 2 Description: This video will show the user going through the screening process but answering “yes” to a question saying that they have had a COVID-19 symptom in the past 48 hours. 

Video 3 Link: https://drive.google.com/file/d/10VXm7ECclt0HnHVIaZO6iedM43bXqnxs/view?usp=sharing

Video 3 Description: The third video shows what happens when there is a normal walkthrough with 3 people already in the room with a max capacity of 5. This person will show no symptoms, increase the room capacity to 80%, turning the LED red. Then the next person trying to enter the room cannot start the process with the LED red and capacity >=80%.

Video 4 Link: https://drive.google.com/file/d/10_hoM85UC7eT6fAjX4yAGA3IksZOznBR/view?usp=sharing


Video 4 Description: The fourth video shows a person trying to leave the room. The function which checks for someone at the door on the inside of the room(within 1.5 feet) is called multiple times throughout the code at various times which are not time-critical such as when waiting for someone to approach the door from the outside, such as between questions being asked for example. For this video, the current occupancy will be set to 4 with a maximum of 5 so that when someone leaves the room at the beginning, the LED will change from red to yellow, and later on, while someone (sparkfun box and notebook in the video) is taking the screening, another person will try to leave and this will change the LED from yellow to green.

Video 5 Link: https://drive.google.com/file/d/10k5OFAyCbj5UsoRugcvrWXlZ04_X6eNh/view?usp=sharing

Video 5 Description: This video just shows that the distance sensor will not register when someone is over 1.5 feet away and, when someone does not answer a question within 60 seconds, it will time out and the screening will reset at the beginning. Feel free to skip to about the 1:30 mark for the actual timeout.

