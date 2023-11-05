# First Order Logic using Resolution Method

Decide the given statement is True or False depending on the given knowledge base, restricted to "For All" Statements. 
The Algorithm converts the given knowledge base into Resolution tree, and using the proof of restitution, we decide if the given statement is true or false. The implementation of this project stands true for a cyclic dependencies as well as paradoxes. 

Example: 
Alert(Alice)
3
Take(x,Ca) => Take(x,VitD)
Take(x,VitD) => Take(x,VitA)
Take(x,VitA) => Take(x,Ca)

Output: 
FALSE