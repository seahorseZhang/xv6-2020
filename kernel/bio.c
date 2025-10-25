// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBucketSize 13
#define NBufSize 5

struct {
  struct spinlock lock;
  struct buf buf[NBufSize];
} bcache[NBucketSize];

void
binit(void)
{
  for (int i = 0; i < NBucketSize; i++) {
    initlock(&bcache[i].lock, "bcache");
    for (int j = 0; j < NBufSize; j++) {
      struct buf *buf = &bcache[i].buf[j];
      initsleeplock(&buf->lock, "sleeplock");
      buf->refcnt = 0;
      buf->valid = 0;
      buf->dev = -1;
      buf->blockno = -1;
    }
  }
}

int getHash(uint dev, uint blockno) {
  return (dev + blockno) % NBucketSize;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int hash = getHash(dev, blockno);
  acquire(&bcache[hash].lock);

  // Is the block already cached?
  for(int i = 0; i < NBufSize; i++){
    b = &bcache[hash].buf[i];
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->lastUsed = ticks;
      release(&bcache[hash].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  int lastUsed = 0xffffffff;
  int lastIndex = -1;
  for(int i = 0; i < NBufSize; i++){
    b = &bcache[hash].buf[i];
    if(b->refcnt == 0 && b->lastUsed < lastUsed) {
      lastUsed = b->lastUsed;
      lastIndex = i;
    }
  }
  if (lastIndex == -1) {
    panic("bget: no buffers");
  }
  b = &bcache[hash].buf[lastIndex];
  b->dev = dev;
  b->blockno = blockno;
  b->refcnt = 1;
  b->valid = 0;
  b->lastUsed = ticks;
  release(&bcache[hash].lock);
  acquiresleep(&b->lock);
  return b;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  int hash = getHash(b->dev, b->blockno);
  acquire(&bcache[hash].lock);
  b->refcnt--;
  if (b->refcnt < 0)
    panic("brelse: refcnt < 0");
  release(&bcache[hash].lock);
  releasesleep(&b->lock);
}

void
bpin(struct buf *b) {
  int hash = getHash(b->dev, b->blockno);
  acquire(&bcache[hash].lock);
  b->refcnt++;
  release(&bcache[hash].lock);
}

void
bunpin(struct buf *b) {
  int hash = getHash(b->dev, b->blockno);
  acquire(&bcache[hash].lock);
  b->refcnt--;
  if (b->refcnt < 0)
    panic("bunpin: refcnt < 0");
  release(&bcache[hash].lock);
}


