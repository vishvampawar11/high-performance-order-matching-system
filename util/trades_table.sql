CREATE TABLE
    trades (
        id SERIAL PRIMARY KEY,
        aggressor_id VARCHAR(50) NOT NULL,
        passive_id VARCHAR(50) NOT NULL,
        price NUMERIC NOT NULL,
        quantity NUMERIC NOT NULL,
        timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );