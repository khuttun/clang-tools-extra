.. title:: clang-tidy - misc-uniqueptr-release-unused-retval

misc-uniqueptr-release-unused-retval
====================================

Warns if the return value of ``std::unique_ptr::release()`` is not used.

Discarding the return value results in leaking the managed object, if the
pointer isn't stored anywhere else. This can happen for example when
``release()`` is incorrectly used instead of ``reset()``:

.. code-block:: c++

  void deleteObject() {
    MyUniquePtr.release();
  }

The check will warn about this. The fix is to replace the ``release()`` call
with ``reset()``.

Discarding the ``release()`` return value doesn't necessary result in a leak if
the pointer is also stored somewhere else:

.. code-block:: c++

  void f(std::unique_ptr<Foo> p) {
    // store the raw pointer
    storePointer(p.get());

    // prevent destroying the Foo object when the unique_ptr is destructed
    p.release();
  }

The check warns also here. Although there's no leak here, the code can still be
improved by using the ``release()`` return value:

.. code-block:: c++

  void f(std::unique_ptr<Foo> p) {
    storePointer(p.release());
  }

This eliminates the possibility that code causing ``f()`` to return, thus
causing ``p``'s destructor to be called and making the stored raw pointer
dangle, is added between ``storePointer()`` and ``release()`` calls.
