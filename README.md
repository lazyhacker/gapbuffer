gapbuffer
=========

C++ implementation of a gap buffer.

The gap buffer is a technique used by text editors to store the entire text in a linear block of memory. It is a fairly simple technique that involves keeping track of 5 pointers and a sequencial block (gap) inside the buffer structure for inserting new text. The five pointers are (1) head of the buffer, (2) start of the gap, (3) first location outside the gap, (4) end of the buffer, and (5) location (point) within the buffer. The main rule for point is that it must be within the buffer and cannot be anywhere inside the gap other than the beginning of it.

When text is to be written, the gap is moved to the point and new text can be written directly (each new character shrinks the gap by one). If there is no more room inside the gap, the gap is expanded. Deleting of a character involves moving the gap to the point and extending it to cover the deleted character.

More details on this technique can be found in Craig Finseth's ["The Craft of Text Editing"](http://www.finseth.com/craft/) which is now available online, and the usenet postings of Joseph Allen. A set of the usenet postings (editech.X.Z) can be downloaded here:

  - [editech.1.Z](http://www.lazyhacker.com/editech.1.Z)
  - [editech.2.Z](http://www.lazyhacker.com/editech.2.Z)
  - [editech.3.Z](http://www.lazyhacker.com/editech.3.Z)
  - [editech.4.Z](http://www.lazyhacker.com/editech.4.Z)
  - [editech.5.Z](http://www.lazyhacker.com/editech.5.Z)

Another very useful text is Ray Valdes' article, ["Text Editors: Algorithm and Architectures"](http://www.drdobbs.com/architecture-and-design/text-editors-algorithms-and-architecture/184408975), published in Dr. Dobb's Journal.

What I've included is the source for a basic C++ implementation of gap buffer class which I hope will be useful to others. I don't have any restrictions on the use of this code other than that you include my name with any derived work. This is just my small contribution to the open source community and I hope to be able to offer more in the future, but for now I'm starting small. Any improvements, suggestions or advice are always appreciated.
