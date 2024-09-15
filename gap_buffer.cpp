/*
 *  gap_buffer.cpp
 * 
 *  Author: Hsin Tsao (stsao@lazyhacker.com)
 *  Version: 1.0 (June 12, 2003) 
 *
 *  This provides the implementation to the GapBuffer class defined
 *  in text_buffer.h.
 *
 *  Portions of this work derived from Joseph Allen's usenet
 *  postings on comp.editor that was released to the public
 *  domain. 
 * 
 *  
 *  There are no restrictions on the use of this code other
 *  than to include my name in any derived work.  There are
 *  no warranty for this obviously, but you are welcomed
 *  to modify, correct, or adapt the code to your needs.  The
 *  author appreciates if you are willing to submit corrections
 *  or suggestions for improvement.
 *  
 *  
 *  http://www.lazyhacker.com
 */ 

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gap_buffer.h"

using namespace std;

GapBuffer::GapBuffer(int gsize) : GAP_SIZE(gsize), buffer(NULL)
{
    InitBuffer(GAP_SIZE);

};


GapBuffer::GapBuffer(FILE *file, int gsize) : GAP_SIZE(gsize)
{
    
    // determine the size of the file then create
    // a buffer of size + GAP_SIZE
    struct stat buf;
    
    fstat(fileno(file), &buf);
    long file_size = buf.st_size;
    InitBuffer(file_size + GAP_SIZE);    
    MoveGapToPoint();
    ExpandGap( (int)file_size );
    unsigned int amount = fread(gapstart, 1, file_size, file);

    gapstart += amount;



};

/*  Copy Constructor - Since we have pointers
 *  as member data, we need to provide our own
 *  copy constructor because the default will
 *  only cause copies to all point to the same
 *  location.
 */
GapBuffer::GapBuffer(const GapBuffer& tb)
{
     GAP_SIZE = tb.GAP_SIZE;
     
     buffer = (char *) malloc(tb.bufend - tb.buffer);
     
     strcpy(buffer, tb.buffer);

     bufend = buffer + (tb.bufend - tb.buffer);
     gapstart = buffer + (tb.gapstart - tb.buffer);
     gapend = gapstart + (tb.gapend - tb.gapstart);
     point = buffer + (tb.point - tb.buffer);
    
}


GapBuffer::~GapBuffer()
{

    if (buffer) {
        free(buffer);
    }

};

/*
 * Copy the characters from one location to another.  We have
 * to write our own instead of using memcopy because we are
 * working within a single linear buffer and thus can have
 * overlap between the source and destination.
 */
int GapBuffer::CopyBytes(char *destination, char *source, unsigned int length)
{

    if ( (destination == source) || (length == 0) ) {
        return 1;
    }

    // if we're moving the character toward the front of the buffer
    if (source > destination) {

        // check to make sure that we don't go beyond the buffer
        if ( (source + length) >= bufend ) {
            return 0;
        }

        for (; length > 0; length--) {
            *(destination++) = *(source++);
        }

    } else {

        // To prevent overwriting characters we still
        // need to move, go to the back and copy forward.
        source += length;
        destination += length;

        for (; length > 0; length--) {
            // decrement first 'cause we start one byte beyond where we want
            *(--destination) = *(--source); 
        }
    }

    return 1;

}

/*
 *  Expand the buffer to new size + GAP_SIZE.
 *  
 */
void GapBuffer::ExpandBuffer(unsigned int size)
{   

    // Check to see that we actually need to increase the buffer
    // since BufferSize doesn't include the gap.
    if ( ( (bufend - buffer) + size) > BufferSize() ) {

        char *origbuffer = buffer;

        int NewBufferSize = (bufend - buffer) + size  + GAP_SIZE;
        
        buffer = (char *) realloc(buffer, NewBufferSize);

        point += buffer - origbuffer;
        bufend += buffer - origbuffer;
        gapstart += buffer - origbuffer;
        gapend += buffer - origbuffer;
    }

}

/*
 * Move the gap to the current position of the point.
 * The point should end in same location as gapstart.
 */
void GapBuffer::MoveGapToPoint()
{


    if (point == gapstart) {
        return;
    }

    if (point == gapend) {
        point = gapstart;
        return;
    }

    // Move gap towards the left 
    if (point < gapstart) {
        // Move the point over by gapsize.        
        CopyBytes(point + (gapend-gapstart), point, gapstart - point);
        gapend -= (gapstart - point);
        gapstart = point;
    } else {
        // Since point is after the gap, find distance
        // between gapend and point and that's how
        // much we move from gapend to gapstart.
        CopyBytes(gapstart, gapend, point - gapend);
        gapstart += point - gapend;
        gapend = point;
        point = gapstart;
    }
}

/*
 *  Expand the size of the gap.  If the required
 *  size is less then the current gap size, do
 *  nothing.  If the size is greater than the 
 *  current size, increase the gap to the default
 *  gap size + size.
 */
void GapBuffer::ExpandGap(unsigned int size)
{


    if (size > SizeOfGap()) {
        size += GAP_SIZE;
        ExpandBuffer(size);
        CopyBytes(gapend+size, gapend, bufend - gapend);

        gapend += size;
        bufend += size;
    }

}

/*
 *  Set point to offset from start of buffer.
 */
void GapBuffer::SetPoint(unsigned int offset)
{

    point = buffer + offset;

    if (point > gapstart) {
        point += gapend - gapstart;
    }

}

/*
 *  Returns the current size of the gap.
 */
int GapBuffer::SizeOfGap()
{
    return gapend - gapstart;

}

/*
 *  Returns offset from point to start of buffer.
 */
unsigned int GapBuffer::PointOffset()
{

    if (point >= gapend) {
        return ((point - buffer) - (gapend - gapstart));
    } else {
        return (point - buffer);
    }
}

/*
 * Return character that point is pointing to.
 * If point is inside the gap, then return the
 * the first character outside the gap.
 */
char GapBuffer::GetChar()
{

    // If the point is anywhere in the gap, then
    // it should always be at the start of the gap.
    if (point == gapstart) {
        point = gapend;
    }

    return *point;
}

/*
 * Return the previous character and
 * move point back one position.
 */
char GapBuffer::PreviousChar()
{
    
    if (point == gapend) {
        point = gapstart;
    }

    return *(--point);
}

/*
 * Replace the character of point.
 */
void GapBuffer::ReplaceChar(char ch)
{

    // Since we're just replacing the current character,
    // we don't need to move or modify the gap.
    if (point == gapstart) {
        point = gapend;
    }

    if (point == bufend) {
        ExpandBuffer(1);
        bufend++;
    }

    *point = ch;
}

/*
 * Increment pointer.  Returns next character.
 */
char GapBuffer::NextChar()
{   
    // point should not be in the gap.
    if (point == gapstart) {
        point = gapend;
    } 

    point++;

    // point should not be in the gap.
    if (point == gapstart) {
        point = gapend;
    }

    return *point;

}

void GapBuffer::PutChar(char ch)
{
    InsertChar(ch);
    *point++;

}

/*
 *  Insert character at point position.  Note
 *  that repeatedly calling this function will
 *  keep calling MoveGapToPoint since this function
 *  doesn't advance the point.  The result is the
 *  text appears reverse of key strokes.
 */
void GapBuffer::InsertChar(char ch)
{
    // Here we do need to move the gap if the point
    // is not already at the start of the gap.

    if (point != gapstart) {
        MoveGapToPoint();
    }

    // check to make sure that the gap has room
    if (gapstart == gapend) {
        ExpandGap(1);
    }

    *(gapstart++) = ch;

}

/*
 *  Delete "size" number of characters.
 */
void GapBuffer::DeleteChars(unsigned int size)
{

    if (point != gapstart) {
        MoveGapToPoint();
    }

    // We shifted the gap so that gapend points to the location
    // where we want to start deleting so extend it 
    // to cover all the characters.
    gapend += size;
}

void GapBuffer::InsertString(const char *string, unsigned int length)
{

    MoveGapToPoint();

    if (length > SizeOfGap()) {
        ExpandGap(length);
    }

    do {
        PutChar(*(string++));
    } while ( length-- );
}

/*
 * Here we initilize the buffer and set
 * the pointers to the correct position.
 */
int GapBuffer::InitBuffer(unsigned int size)
{

    if (buffer) {
        free(buffer);
    }

    buffer = (char *) malloc(size);

    if (!buffer) {
        return 0;
    }

    point = buffer;
    gapstart = buffer;
    
    // initially gapend is outside of buffer
    gapend = buffer + size;     
    bufend = gapend;

    return 1;

}

int GapBuffer::BufferSize()
{
    return (bufend - buffer) - (gapend - gapstart);
}

/*
char* GapBuffer::GetBuffer()
{

    return buffer;
}
*/

void GapBuffer::PrintBuffer()
{
    /*
    char ch;

    cout << "Printing the buffer: " << endl;
    SetPoint(0);
    while (point < bufend) {
        cout << GetCharMovePoint();        
    }

    cout << "Printing the buffer in reverse: " << endl;

    while (point >= buffer) {
        cout << GetPrevCharMovePoint();
    }
    */

    char *temp = buffer;


    while (temp < bufend) {

        if ( (temp >= gapstart) && (temp < gapend) ) {
            cout << "_";
            temp++;
        } else {
            cout << *(temp++);
        }

    }
    cout << endl;
}

int GapBuffer::SaveBufferToFile(FILE *file, unsigned int bytes)
{

    if (!bytes) {
        return 1;
    }

    if (point == gapstart) {
        point = gapend;
    }

    if ( (gapstart > point) && (gapstart < (point + bytes)) && (gapstart != gapend) ) {
        if ( gapstart - point != fwrite(point, 1, gapstart-point, file) ) {
            return 0;
        }

        if ( (bytes - (gapstart - point)) != fwrite(gapend, 1, bytes-(gapstart - point), file) ) {
            return 1;
        }

        return 1;
    } else {
        return bytes == fwrite(point, 1, bytes, file);
    }


}
