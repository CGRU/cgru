.. _afanasy-tickets:

=======
Tickets
=======

Ticket is some named counter.

Pool can have *host* and *pool* tickets.
*Host* means that each pool host has such tickets.
*Pool* means that an entire pool has this tickets.

If job block has tickets it can run only on pools and renders that has such tickets enough.
Each block task will be produced with block tickets.
When render starting task with tickets, it counts tickets usage.
Pool counts total tickets usage.

For example if you want to limit licenses on the entire farm,
you can set ``NUKE:20`` *pool* tickets on the root pool.
If you want to limit RAM usage on each pool host,
You can set ``MEM:64`` *host* tickets on a pool which renders has 64GB RAM.
And each pool render will be able to run one task with ``MEM:64`` tickets,
or 2 tasks with ``MEM:32``, or 1 ``MEM:32`` and 3 ``MEM:10`` tasks at the same time.
Just one side effect will appear in this case,
pools will count total ``MEM`` tickets and renders will count ``NUKE`` tickets too.

Ticket can be displayed as some custom image,
if a png file with the same name exists in the direcory:

``cgru/icons/tickets``

