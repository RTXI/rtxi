Welcome to RTXI!
====

<a href="http://www.rtxi.org">The Real-Time eXperiment Interface (RTXI)</a> is a collaborative open-source software development project aimed at producing a real-time Linux based software system for hard real-time data acquisition and control applications in biological research.

RTXI uses the open source <a href="http://xenomai.org">Xenomai framework</a> to implement communication with a variety of commercially available multifunction DAQ cards with both analog and digital input and output channels. This makes RTXI essentially hardware-agnostic and able to communicate with multiple actuators and sensors that may span different modalities.

At the heart of RTXI is a dynamic framework allowing for easy creation and use of modules. Each module features its own encapsulated interface through which users can control module execution and modify its various parameters. Modules contain function-specific code that can be used in combinations to build custom workflows and experiment protocols. They are compiled outside the core RTXI source tree as shared object libraries that are linked at runtime. Take a look <a href="http://www.rtxi.org/topics/modules/">here</a> for more information on the architecture of RTXI and <a href="https://github.com/RTXI/modules">here</a> for modules used by other labs.

If you encounter problems or have questions, please refer to the <a href="https://github.com/RTXI/rtxi/wiki">wiki</a> or post them to the <a href="https://github.com/RTXI/rtxi/issues">issues section</a> and we'll do our best to help get you started.

To see who/how RTXI is being used around the world and to stay up-to-date, please visit our <a href="http://www.rtxi.org">website</a>.
