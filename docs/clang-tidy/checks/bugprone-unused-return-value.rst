.. title:: clang-tidy - bugprone-unused-return-value

bugprone-unused-return-value
============================

Warns on unused function return values. The checked funtions can be configured.

Options
-------

.. option:: FunctionsRegex

   A regular expression specifying the functions to check. Defaults to
   ``^::std::async$|^::std::launder$|^::std::remove$|^::std::remove_if$|^::std::unique$|^::std::unique_ptr<.*>::release$|^::std::.*::allocate$|^::std::(.*::)*empty$``.
   This means that the calls to following functions are checked by default:

   - ``std::async()``. Not using the return value makes the call synchronous.
   - ``std::launder()``. Not using the return value usually means that the
     function interface was misunderstood by the programmer. Only the returned
     pointer is "laundered", not the argument.
   - ``std::remove()``, ``std::remove_if()`` and ``std::unique()``. The returned
     iterator indicates the boundary between elements to keep and elements to be
     removed. Not using the return value means that the information about which
     elements to remove is lost.
   - ``std::unique_ptr::release()``. Not using the return value can lead to
     resource leaks if the same pointer isn't stored anywhere else. Often,
     ignoring the ``release()`` return value indicates that the programmer
     confused the function with ``reset()``.
   - All ``allocate()`` calls in ``std``. For example
     ``std::allocator::allocate()`` and
     ``std::pmr::memory_resource::allocate()``. Not using the return value is a
     resource leak.
   - All ``empty()`` calls in ``std``. For example ``std::vector::empty()``,
     ``std::filesystem::path::empty()`` and ``std::empty()``. Not using the
     return value often indicates that the programmer confused the function with
     ``clear()``.
