#ifndef O2CONT_H
#define O2CONT_H

#include <TBuffer.h>
#include <TObject.h>
#include <memory>
#include <algorithm>
#include <type_traits>

//using bufType = std::int8_t;
///using sizeType = std::int32_t;
typedef std::int8_t bufType;
typedef std::int32_t sizeType;

template <class T, class H>
  class Container: public TObject {

 public:
  
  struct Header {
    H         userInfo;         // user assigned data info
    int       expandPolicy;     // user assigned policy: n>0 -> new=old+n, n<=0 -> new=2*(old+n)  
    sizeType  nObjects;         // number of objects stored
  };

  // main constructor
  Container(sizeType iniSize=0, int expPol=-100);
  
  /// set/get data info (user defined)
  void  setUserInfo(const H& val);
  H&    getUserInfo()            const {return getHeader()->userInfo;}

  /// set/get expand policy, see constructor documentation
  void  setExpandPolicy(int val)       {getHeader()->expandPolicy = val;}
  int   getExpandPolicy()        const {return getHeader()->expandPolicy;}

  /// get number of objects stored
  sizeType size()                const {return getHeader()->nObjects;}

  /// get currently booked capacity
  sizeType capacity()            const {return (mBooked-dataOffset)/sizeof(T);}

  /// get i-th object pointer w/o boundary check
  T*    operator[](sizeType i)   const {return getData()+i;}

  /// get i-th object pointer with boundary check
  T*    at(sizeType i)           const {return i<size() ? (*this)[i] : nullptr;}

  /// get last object pointer
  T*    back()                   const {return size() ? (*this)[size()-1] : nullptr;}

  /// get 1st object pointer
  T*    front()                  const {return size() ? (*this)[0] : nullptr;}
  
  /// add copy of existing object to the end, return created copy pointer
  T*    push_back(const T& obj);

  /// create an object with supplied arguments in the end of the container
  template<typename ...Args> T* emplace_back(Args&&... args);
  
  /// clear content w/o changing capacity, if requested, explicitly delete objects
  void  clear(bool calldestructor=true);

  /// book space for objects and aux data
  void  reserve(sizeType n=1000);

  /// expand space for new objects
  void  expand();

  /// get raw pointer
  bufType*            getPtr()    const {return mPtr.get();}

 protected:

  /// cast the head of container buffer to internal layout
  Header*             getHeader() const {return reinterpret_cast<Header*>(getPtr());}
  T*                  getData()   const {return reinterpret_cast<T*>(getPtr()+dataOffset);}
  /// return next free slot, for "new with placement" creation, not LValue
  T*                  nextFreeSlot();
  
  // offset of data wrt buffer start: account for Header + padding after Header block to respect T alignment
  constexpr static sizeType dataOffset = sizeof(Header) + sizeof(Header)%alignof(T); 

  /// get pointer on the container
  // std::unique_ptr<bufType> getUPtr()  const {return mPtr;}
  
 protected:
  
  sizeType mBooked;                 ///< number of words booked, used for persistency only
  std::unique_ptr<bufType[]> mPtr;  //->  ///[mBooked] pointer on continuos block containing full object data

  ClassDef(Container,1)
};



//-------------------------------------------------------------------
template <class T, class H>
  Container<T,H>::Container(sizeType iniSize, int expPol) : mBooked(0), mPtr()
  {
    /**
     * Creates container for objects of 
     * T POD class
     * with single description field of H POD class
     * size: initial capacity of the container
     * expPol: expansion policy. new_size = expPol>0 ? oldSize+expPol : 2*max(1,oldSize+|expPol|)
     */
    static_assert(std::is_trivially_copyable<T>::value,"Object class is not trivially-copiable");
    static_assert(std::is_trivially_copyable<H>::value,"Header class is not trivially-copiable");
    reserve(iniSize);
    if (!expPol) expPol = -1;
    getHeader()->expandPolicy = expPol;
  }

//-------------------------------------------------------------------
template <class T, class H>
  void Container<T,H>::reserve(sizeType n)
{
  /**
   * Resize container to size n. 
   * Existing content is preserved, but truncated to n if n<current_n_objects
   */
  if (n<0) n = 0;
  sizeType bookSize = dataOffset + n*sizeof(T);
  //  auto tmpPtr = std::move(mPtr);
  std::unique_ptr<bufType[]> tmpPtr(new bufType[bookSize]());
  mPtr.swap(tmpPtr);
  std::copy(tmpPtr.get(), tmpPtr.get()+std::min(bookSize,mBooked), mPtr.get());
  if (n<getHeader()->nObjects) getHeader()->nObjects = n;
  mBooked = bookSize;
}

//-------------------------------------------------------------------
template <class T, class H>
  void Container<T,H>::setUserInfo(const H& v)
{
  /**
   * Sets data identification field
   */
  getHeader()->userInfo = v;
  //  std::copy_n(&v,1, &getHeader()->userInfo);
}

//-------------------------------------------------------------------
template <class T, class H>
  void Container<T,H>::expand()
{
  /**
   * expand container according to expansion policy
   */
  sizeType oldSize = capacity();
  sizeType newSize = getExpandPolicy()<0 ? 2*std::max(oldSize-getExpandPolicy(),1) : oldSize+getExpandPolicy();
  reserve(newSize);
}

//-------------------------------------------------------------------
template <class T, class H>
  void Container<T,H>::clear(bool calldestructor)
{
  /**
   * clear content w/o changing capacity, if requested, explicitly delete objects
   */
  if (calldestructor) {
    T *objB = back(), *objF = front();
    while (objB>=objF) {
      objB->~T();
      objB--;
    }
  }
  getHeader()->nObjects = 0;
}

//-------------------------------------------------------------------
template <class T, class H>
  inline T* Container<T,H>::nextFreeSlot()
{
  /**
   * Return pointer on next free slot where new object will be placed.
   * If needed, autoexpands 
   */  
  if (size()==capacity()) expand();
  return (*this)[size()];
}

//-------------------------------------------------------------------
template <class T, class H>
  T* Container<T,H>::push_back(const T& obj)
{
  /**
   * Create copy of the object in the end of the container
   * Return pointer on new object
   */
  T* slot = nextFreeSlot();
  new(slot) T(obj);
  //std::copy_n(&obj,1, slot);
  getHeader()->nObjects++;
  return slot;
}

//-------------------------------------------------------------------
template<class T, class H> template <typename ...Args> T* Container<T,H>::emplace_back(Args&&... args) 
{
  /**
   * Create copy of the object in the end of the container
   * Return pointer on new object
   */
  T* slot = nextFreeSlot();
  new(slot) T(std::forward<Args>(args)...);
  getHeader()->nObjects++;
  return slot;
}

/*
//-------------------------------------------------------------------
template<class T, class H>
  void Container<T,H>::Streamer(TBuffer &b)
{
   if (b.IsReading()) {
     Int_t n;
     b >> n;
     mBooked = n;
     char* tmp=0;
     b.ReadFastArray(tmp,n);
     mPtr.reset((bufType*)tmp);
   } else {
     b << int(mBooked);
     b.WriteFastArray((char*)mPtr.get(), int(mBooked));
   }
}
*/
#endif
