Collaboration Plan

This project brings together international student teams to model, simulate, implement, and control a Reaction Wheel Inverted Pendulum (RWIP). The goal is to explore both technical and collaborative challenges in control engineering through simulation and physical implementation.

---

**Week 1: Kickoff & Icebreakers**
- Meet your international teammates (via Zoom or Teams).
- Participate in structured icebreaker activities.
- Share backgrounds, expectations, and technical interests.
- Set up collaborative tools: VS Code, Python, GitHub, shared folders.
- Begin reading reference material on inverted pendulums and reaction wheels.

💡 _Tip: Keep a shared team logbook and schedule weekly check-ins._

---

**Week 2: System Understanding & Parameter Assignment**
- Study the inverted pendulum and reaction wheel system.
- Identify key physical and control concepts: torque, moment of inertia, stability.
- Assign unique parameter sets to each team (e.g., pendulum length, mass, motor power, wheel inertia).
- Clarify technical goals and system constraints.
- Start drafting control objectives.

---

**Week 3: Simulation & Strategy Development**
- Build Python simulations of the RWIP system.
- Explore system dynamics (e.g., swing-up vs. stabilization).
- Identify stable and unstable parameter regions.
- Test control strategies:
  - Open-loop behavior
  - Swing-up with **bang-bang control**
  - Stabilization using **PID** and **LQR** controllers
- Compare performance metrics across teams.

---

**Week 4: Physical Implementation & Integration**
- Assemble the RWIP using available materials (e.g., 3D printed parts, hardware components).
- Integrate sensors (e.g., AS5600 magnetic sensor) and actuators (brushed DC motor).
- Wire and interface the system with Arduino or compatible microcontroller.
- Characterize physical properties: friction, inertia, motor constants.
- Validate component specs vs. datasheets.

---

**Week 5: Testing, Comparison & Analysis**
- Deploy controllers on the Arduino (migrating from Python simulations).
- Test swing-up and stabilization phases in real conditions.
- Compare experimental results with simulated predictions.
- Measure performance: response time, overshoot, stability margins.
- Evaluate control effectiveness and hardware limitations.

---

**Final Session: Reflection & Presentation**
- Present each team’s approach, implementation, and results.
- Reflect on the technical, intercultural, and collaborative experiences.
- Share lessons learned and insights into system limitations.
- Discuss real-world relevance and next steps for further development.

**Optional Deliverables**
Each team should maintain:
- Simulation notebooks (.ipynb)
- Arduino code (.ino)
- CAD files (.stl, .step)
- Data logs (.csv)
- Diagrams, images, and demo videos (.png, .mp4)
- -Description of the system and control strategy  
- Simulations and analysis  
- Photographs and diagrams  
- Electrical and mechanical schematics  
- STL files for 3D parts
- Software architecture diagrams  
- Source code (Python and Arduino)
- Description of tests and results  
- Short video of system in action  
- Reflections and conclusions

Include all in your team’s final GitHub repository.