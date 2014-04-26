-- upgrade script for server repo from OILB to OILC format
-- BEFORE EXECUTION check that application_id equals to 1112295759

-- rename old table revs to temporary name
ALTER TABLE revs RENAME TO revs_old;

-- create new table revs
CREATE TABLE IF NOT EXISTS revs (
	rev INTEGER PRIMARY KEY AUTOINCREMENT,
	date INTEGER NOT NULL,
	user INTEGER NOT NULL,
	latest INTEGER NOT NULL,
	key BLOB NOT NULL,
	value BLOB NOT NULL);

-- insert old data to new table
INSERT INTO revs (rev, date, user, latest, key, value)
	SELECT t1.rev, t1.date, t1.user, t1.rev = t2.maxrev, t1.key, t1.value FROM revs_old AS t1 INNER JOIN (SELECT key, MAX(rev) AS maxrev FROM revs_old GROUP BY key) AS t2 ON t1.key = t2.key;

-- drop old table
DROP TABLE revs_old;

-- set new application id
PRAGMA application_id = 1129072975;

-- compress database
VACUUM;
