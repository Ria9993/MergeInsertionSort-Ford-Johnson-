#pragma once 

#include <cassert>
#include <cstddef>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <climits>

// Enable AVX optimization
#define FLAG_OPTIMIZE_AVX 1
#define AVX_BYTE 32
#define AVX_BYTE_MASK (AVX_BYTE - 1)
#define AVX_NUM_PACKED_PTR (AVX_BYTE / sizeof(Block*))

// ---------------------------------------------------------------------------
//                              Template Helpers
// ---------------------------------------------------------------------------
// __enable_if__
template<bool B, class T = void>
struct __enable_if__ {};

template<class T>
struct __enable_if__<true, T> { typedef T type; };

// __is_same__
template<class T, class U>
struct __is_same__ { static const bool value = false; };

template<class T>
struct __is_same__<T, T> { static const bool value = true; };


// ---------------------------------------------------------------------------
//                              PmergeMe Class
// ---------------------------------------------------------------------------
// Official support container: std::vector, std::deque 
template <typename Container, typename = typename __enable_if__<__is_same__<typename Container::iterator::iterator_category, std::random_access_iterator_tag>::value>::type>
class PmergeMe
{
// Constructors
private:
   PmergeMe(const PmergeMe& other) __attribute__((unused));
public:
    PmergeMe();
    ~PmergeMe();

// Operators Overload
private:
    PmergeMe& operator=(const PmergeMe& other)  __attribute__((unused, noreturn));

// Member Functions
public:
    void sort(Container& container);
};


// ---------------------------------------------------------------------------
//                              Definitions
// ---------------------------------------------------------------------------
template <typename Container, typename HaveRandomIterator>
PmergeMe<Container, HaveRandomIterator>::PmergeMe()
{
}

template <typename Container, typename HaveRandomIterator>
PmergeMe<Container, HaveRandomIterator>::PmergeMe(const PmergeMe& other __attribute__((unused)))
{
}

template <typename Container, typename HaveRandomIterator>
PmergeMe<Container, HaveRandomIterator>::~PmergeMe()
{
}

template <typename Container, typename HaveRandomIterator>
PmergeMe<Container, HaveRandomIterator>& PmergeMe<Container, HaveRandomIterator>::operator=(const PmergeMe& other __attribute__((unused)))
{
    assert(false);
}

template <typename Container, typename HaveRandomIterator>
void PmergeMe<Container, HaveRandomIterator>::sort(Container& container)
{
    if (container.size() <= 1)
    {
        return;
    }

    // Utility macros
    // Round up to the nearest power of 2. If x == 2^N, Then return x.
#define ROUND_UP_POW2(x) ((x) == 0 ? 1 : (1 << (sizeof(size_t) * CHAR_BIT - __builtin_clzl((x) - 1)))) //< __builtin_clzl() == BSF(Bit Scan Forward)

    // Define Block for managing sorted blocks (like a binary tree)
    typedef struct _Block Block;
    typedef struct _Block {
        Block* left;
        Block* right;
        typename Container::value_type keyValue; //< the value of highest element that is contained in sub-blocks
    } Block;

    // Create fixed memory pool for Block allocation
    const size_t containerSizePow2 = ROUND_UP_POW2(container.size());
    Block* blockPool = new Block[(2 * containerSizePow2) + (AVX_BYTE / sizeof(Block*))];
    size_t blockPoolIndex = 0;
#define ALLOCATE_BLOCK() (Block*)(blockPool + blockPoolIndex++)

    // The address of the block will be aligned to 8-byte boundary, so the lower 3 bits are not used.
    // So, use the lower 3 bits to indicate whether the block is disassembled.
#define MARK_BLOCK_AS_DISASSEMBLED(ptr)     reinterpret_cast<Block*>(reinterpret_cast<uintptr_t>(ptr) | 0x01)
#define UNMARK_BLOCK_AS_DISASSEMBLED(ptr)   reinterpret_cast<Block*>(reinterpret_cast<uintptr_t>(ptr) & ~0x01)
#define IS_BLOCK_DISASSEMBLED(ptr)          (reinterpret_cast<uintptr_t>(ptr) & 0x01)

    // Create space for sorting blocks
    Block** arr = new Block*[containerSizePow2 + (AVX_BYTE / sizeof(Block*))];
    
    // Phase 0: create leaf blocks
    for (size_t i = 0; i < container.size(); ++i)
    {
        arr[i] = ALLOCATE_BLOCK();
        arr[i]->left = NULL;
        arr[i]->right = NULL;
        arr[i]->keyValue = container[i];
    }
    arr[container.size()] = NULL;

    // Phase 1: Build the block tree by recursively merging two blocks at a time.
    size_t pairSize;
    for (pairSize = 2; pairSize / 2 < container.size(); pairSize *= 2)
    {
        
        // Sort two block by keyValue and merge to parent block
        size_t i;
        for (i = 0; i * pairSize < container.size(); i++)
        {
            Block* const blockA = arr[(i * 2)];
            Block* const blockB = arr[(i * 2) + 1];
            Block* const merged = ALLOCATE_BLOCK();
            assert(blockA != NULL);
            assert(blockA != NULL || blockB != NULL);

           if (blockB == NULL)
            {
                assert(blockA != NULL);
                merged->left = NULL;
                merged->right = blockA;
                merged->keyValue = blockA->keyValue;
            }
            else
            {
                if (blockA->keyValue < blockB->keyValue)
                {
                    merged->left = blockA;
                    merged->right = blockB;
                    merged->keyValue = blockB->keyValue;
                }
                else
                {
                    merged->left = blockB;
                    merged->right = blockA;
                    merged->keyValue = blockA->keyValue;
                }
            }

            assert(merged->left == NULL || merged->left->keyValue >= 0);
            assert(merged->right == NULL || merged->right->keyValue >= 0);

            arr[i] = merged;
        }

        assert(i != 0);
        arr[i] = NULL;
    }

    // Phase 2: Disassemble the blocks into chlid blocks and merge them.
    //          The merge process is performed by inserting in the order of the jacobsthal number.  
    //          and recursively do this until there is only leaf blocks left.
    pairSize /= 2;
    size_t arrSize = 1;
    for (; pairSize >= 2; pairSize /= 2)
    {
        size_t arrShiftedSize = 0;
        size_t jacobsN = 1;
        size_t jacobsPrevN = 0;
        while (arrSize > jacobsPrevN + arrShiftedSize)
        {
            // Disassemble the block into two blocks and Insert them into the array by jacobsthal number order.
            // (jacobsN is one based index. So, subtract 1 to get zero based index.)
            size_t localArrShiftedSize = arrShiftedSize;
            for (size_t i = jacobsN - 1; i >= jacobsPrevN; i--)
            {
                if (i + arrShiftedSize >= arrSize)
                {
                    i = arrSize - arrShiftedSize;
                    continue;
                }

                // Select the block which is not disassembled
                size_t searchCount = 0;
                Block* block = arr[i + localArrShiftedSize];
                while (IS_BLOCK_DISASSEMBLED(block))
                {
                    arr[i + localArrShiftedSize] = UNMARK_BLOCK_AS_DISASSEMBLED(block);
                    assert(localArrShiftedSize > 0);
                    localArrShiftedSize -= 1;
                    block = arr[i + localArrShiftedSize + searchCount * 2];
                    searchCount++;
                }
                Block* const leftBlock = block->left;
                Block* const rightBlock = block->right;
                assert (leftBlock == NULL || leftBlock->keyValue >= 0);
                assert (rightBlock == NULL || rightBlock->keyValue >= 0);
                // DEALLOCATE_BLOCK(block); //< Not necessary. Disable in release build for performance.

                // Insert the disassembled blocks
                if (leftBlock == NULL)
                {
                    // Insert right block
                    assert(rightBlock != NULL);
                    arr[i + localArrShiftedSize] = rightBlock;
                }
                else {
                    // Insert right block
                    assert(rightBlock != NULL);
                    arr[i + localArrShiftedSize] = rightBlock;

                    // Find the insertion point of the left block using binary search
                    size_t right = i + localArrShiftedSize;
                    size_t left = 0;
                    while (left < right)
                    {
                        const size_t mid = (left + right) / 2;
                        const typename Container::value_type midKeyValue = UNMARK_BLOCK_AS_DISASSEMBLED(arr[mid])->keyValue;
                        if (midKeyValue < leftBlock->keyValue)
                        {
                            left = mid + 1;
                        }
                        else
                        {
                            right = mid;
                        }
                    }
                    const size_t insertIndex = left;

                    // Shift the array to make space for inserting the left block
                    for (size_t j = arrSize; j > insertIndex; j--)
                    {
                        arr[j] = arr[j - 1];
                    }

                    // Insert left block
                    assert(insertIndex < arrSize);
                    arr[insertIndex] = MARK_BLOCK_AS_DISASSEMBLED(leftBlock);
                    
                    localArrShiftedSize += 1;
                    arrShiftedSize += 1;
                    arrSize += 1;
                }
                
                // If jacobsN is 1, it should loop only once.
                // (But because of the nature of the Jacobsthal sequence, it should be handled separately.)
                if (jacobsN == 1) {
                    jacobsPrevN = 1;
                    break;
                }
            }

            // Update Jacobsthal number
            const size_t jacobsNextN = jacobsN + 2 * jacobsPrevN;
            jacobsPrevN = jacobsN;
            jacobsN = jacobsNextN;
        }

        // Unmark the disassembled blocks for the next stage
        for (size_t j = 0; j < arrSize; j++)
        {
            arr[j] = UNMARK_BLOCK_AS_DISASSEMBLED(arr[j]);
        }
    }

    // Phase 3: Apply the result
    for (size_t i = 0; i < container.size(); i++)
    {
        container[i] = arr[i]->keyValue;
    }

    // Cleanup
    delete[] arr;
    arr = NULL;
    delete[] blockPool;
    blockPool = NULL;

    // Undefine macros
#undef ALLOCATE_BLOCK
#undef MARK_BLOCK_AS_DISASSEMBLED
#undef UNMARK_BLOCK_AS_DISASSEMBLED
#undef IS_BLOCK_DISASSEMBLED
}
