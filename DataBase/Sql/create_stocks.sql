CREATE TABLE public.stocks (
  figi CHAR(12) NOT NULL,
  "interval" CHAR(5) NOT NULL,
  "time" TIMESTAMP WITHOUT TIME ZONE NOT NULL,
  open REAL NOT NULL,
  close REAL NOT NULL,
  high REAL NOT NULL,
  low REAL NOT NULL,
  volume BIGINT DEFAULT 0,
  CONSTRAINT stocks_pkey PRIMARY KEY(figi, "interval", "time")
) 
WITH (oids = false);

ALTER TABLE public.stocks
  OWNER TO postgres;