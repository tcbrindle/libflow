
.. _glossary:

Glossary
========

.. glossary::
  :sorted:

  flow
    A sequence of :term:`item`\ s

  item
    A single element of a :term:`flow`

  item type
    The C++ type of the items of a flow. May be an object or reference type.
    The item type of a flow `F` can be found with `flow::item_t<F>`

    Example::

      auto f = flow::from(std::vector{1, 2, 3});
      using I = item_t<decltype(f)>;
      static_assert(std::is_same_v<I, int&>); // This flow's item type is reference-to-int

    See also: :term:`value type`

  source
    The start of a pipeline

  value type
    The C++ type