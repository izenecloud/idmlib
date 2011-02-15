select page.page_title,redirect.rd_title from page,redirect where page.page_id=redirect.rd_from order by redirect.rd_title limit 0,10;
