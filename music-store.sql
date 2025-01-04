--
-- PostgreSQL database dump
--

-- Dumped from database version 17.2
-- Dumped by pg_dump version 17.2

-- Started on 2025-01-04 17:25:31

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- TOC entry 226 (class 1259 OID 25692)
-- Name: admins; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.admins (
    id integer NOT NULL,
    username character varying(50) NOT NULL,
    password character varying(100) NOT NULL
);


ALTER TABLE public.admins OWNER TO postgres;

--
-- TOC entry 225 (class 1259 OID 25691)
-- Name: administrators_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.administrators_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.administrators_id_seq OWNER TO postgres;

--
-- TOC entry 4898 (class 0 OID 0)
-- Dependencies: 225
-- Name: administrators_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.administrators_id_seq OWNED BY public.admins.id;


--
-- TOC entry 222 (class 1259 OID 25665)
-- Name: clients; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.clients (
    id integer NOT NULL,
    first_name character varying(50) NOT NULL,
    last_name character varying(50) NOT NULL,
    phone character varying(15) NOT NULL,
    email character varying(100) NOT NULL,
    password character varying(100) NOT NULL
);


ALTER TABLE public.clients OWNER TO postgres;

--
-- TOC entry 221 (class 1259 OID 25664)
-- Name: clients_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.clients_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.clients_id_seq OWNER TO postgres;

--
-- TOC entry 4899 (class 0 OID 0)
-- Dependencies: 221
-- Name: clients_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.clients_id_seq OWNED BY public.clients.id;


--
-- TOC entry 218 (class 1259 OID 25644)
-- Name: instrument_types; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.instrument_types (
    id integer NOT NULL,
    name character varying(50) NOT NULL
);


ALTER TABLE public.instrument_types OWNER TO postgres;

--
-- TOC entry 217 (class 1259 OID 25643)
-- Name: instrument_types_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.instrument_types_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.instrument_types_id_seq OWNER TO postgres;

--
-- TOC entry 4900 (class 0 OID 0)
-- Dependencies: 217
-- Name: instrument_types_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.instrument_types_id_seq OWNED BY public.instrument_types.id;


--
-- TOC entry 220 (class 1259 OID 25651)
-- Name: instruments; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.instruments (
    id integer NOT NULL,
    name character varying(100) NOT NULL,
    type_id integer NOT NULL,
    price numeric(15,0) NOT NULL,
    description text,
    image_path character varying(255)
);


ALTER TABLE public.instruments OWNER TO postgres;

--
-- TOC entry 219 (class 1259 OID 25650)
-- Name: instruments_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.instruments_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.instruments_id_seq OWNER TO postgres;

--
-- TOC entry 4901 (class 0 OID 0)
-- Dependencies: 219
-- Name: instruments_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.instruments_id_seq OWNED BY public.instruments.id;


--
-- TOC entry 224 (class 1259 OID 25674)
-- Name: purchases; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.purchases (
    id integer NOT NULL,
    client_id integer NOT NULL,
    instrument_id integer NOT NULL,
    purchase_date timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.purchases OWNER TO postgres;

--
-- TOC entry 223 (class 1259 OID 25673)
-- Name: purchases_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.purchases_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.purchases_id_seq OWNER TO postgres;

--
-- TOC entry 4902 (class 0 OID 0)
-- Dependencies: 223
-- Name: purchases_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.purchases_id_seq OWNED BY public.purchases.id;


--
-- TOC entry 4720 (class 2604 OID 25695)
-- Name: admins id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins ALTER COLUMN id SET DEFAULT nextval('public.administrators_id_seq'::regclass);


--
-- TOC entry 4717 (class 2604 OID 25668)
-- Name: clients id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients ALTER COLUMN id SET DEFAULT nextval('public.clients_id_seq'::regclass);


--
-- TOC entry 4715 (class 2604 OID 25647)
-- Name: instrument_types id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instrument_types ALTER COLUMN id SET DEFAULT nextval('public.instrument_types_id_seq'::regclass);


--
-- TOC entry 4716 (class 2604 OID 25654)
-- Name: instruments id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instruments ALTER COLUMN id SET DEFAULT nextval('public.instruments_id_seq'::regclass);


--
-- TOC entry 4718 (class 2604 OID 25677)
-- Name: purchases id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases ALTER COLUMN id SET DEFAULT nextval('public.purchases_id_seq'::regclass);


--
-- TOC entry 4892 (class 0 OID 25692)
-- Dependencies: 226
-- Data for Name: admins; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.admins VALUES (1, 'admin1', 'adminpass1');
INSERT INTO public.admins VALUES (2, 'admin2', 'adminpass2');


--
-- TOC entry 4888 (class 0 OID 25665)
-- Dependencies: 222
-- Data for Name: clients; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.clients VALUES (1, 'Иван', 'Боздунов', '123-456-7890', 'ivan.bozdunov@example.com', 'password123');
INSERT INTO public.clients VALUES (2, 'Ильдар', 'Попов', '987-654-3210', 'ildar.popov@example.com', 'securepass');
INSERT INTO public.clients VALUES (3, 'Марк', 'Кирдянов', '555-123-4567', 'mark.kirdyanov@example.com', 'alicepass');
INSERT INTO public.clients VALUES (4, 'Даниил', 'Летяев', '444-555-6666', 'daniil.letyaev@example.com', 'bobsecure');
INSERT INTO public.clients VALUES (5, 'Артур', 'Газизов', '333-222-1111', 'artur.gazizov@example.com', 'charliekey');
INSERT INTO public.clients VALUES (6, 'Константин', 'Андреев', '777-888-9999', 'konstantin.andreev@example.com', 'emilypass');


--
-- TOC entry 4884 (class 0 OID 25644)
-- Dependencies: 218
-- Data for Name: instrument_types; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.instrument_types VALUES (1, 'Электрогитары');
INSERT INTO public.instrument_types VALUES (2, 'Бас-гитары');
INSERT INTO public.instrument_types VALUES (3, 'Усилители');
INSERT INTO public.instrument_types VALUES (4, 'Струны');
INSERT INTO public.instrument_types VALUES (6, 'Клавишные');
INSERT INTO public.instrument_types VALUES (7, 'Гарнитуры');
INSERT INTO public.instrument_types VALUES (5, 'Ударные');


--
-- TOC entry 4886 (class 0 OID 25651)
-- Dependencies: 220
-- Data for Name: instruments; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.instruments VALUES (43, 'IBANEZ IACS61C', 4, 890, 'Металлические струны', 'strings');
INSERT INTO public.instruments VALUES (44, 'IBANEZ IEGS6', 4, 690, 'Металлические струны', 'strings');
INSERT INTO public.instruments VALUES (45, 'IBANEZ IEGS61', 4, 790, 'Струны для электрогитары', 'strings');
INSERT INTO public.instruments VALUES (46, 'IBANEZ IEGS6HG', 4, 790, 'Струны для электрогитары', 'strings');
INSERT INTO public.instruments VALUES (47, 'SHURE SRH440A', 7, 15490, 'Студийные/мониторные наушники', 'headsets');
INSERT INTO public.instruments VALUES (48, 'SHURE SRH840A-EFS', 7, 23990, 'Студийные/мониторные наушники', 'headsets');
INSERT INTO public.instruments VALUES (49, 'Beyerdynamic DT 770 PRO 250 ohms', 7, 25990, 'Студийные/мониторные наушники', 'headsets');
INSERT INTO public.instruments VALUES (50, 'Beyerdynamic DT 990 PRO 250 ohms', 7, 27990, 'Студийные/мониторные наушники', 'headsets');
INSERT INTO public.instruments VALUES (1, 'IBANEZ GRGR131EX-BKF', 1, 34010, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (2, 'IBANEZ GRGR221PA-AQB', 1, 42560, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (3, 'IBANEZ GRX70QA-TRB', 1, 32120, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (4, 'ROCKDALE Akira EX Reverse Snow White', 1, 19100, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (7, 'Solar X2.6C', 1, 162990, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (8, 'Solar X1.6 Canibalismo+', 1, 239999, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (9, 'ROCKDALE Stars HSS Blue Metallic', 1, 11200, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (10, 'Solar A2.6CAR', 1, 122990, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (11, 'Solar A2.6FRPN Canibalismo', 1, 135990, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (12, 'IBANEZ RGR752AHBF-WK', 1, 226790, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (13, 'IBANEZ RGDR4427FX-NTF', 1, 296090, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (15, 'IBANEZ GSR200B-WNF', 2, 40990, '4-струнная бас-гитара', 'bassguitars');
INSERT INTO public.instruments VALUES (16, 'CORT Action-Bass-Plus-LH-BK Action Series', 2, 26690, '4-струнная бас-гитара', 'bassguitars');
INSERT INTO public.instruments VALUES (17, 'STERLING StingRay Blue Sparkle', 2, 168000, '4-струнная бас-гитара', 'bassguitars');
INSERT INTO public.instruments VALUES (18, 'CORT A5-Ultra-Ash-WCASE-ENB Artisan Series', 2, 127730, '5-струнная бас-гитара', 'bassguitars');
INSERT INTO public.instruments VALUES (14, 'IBANEZ GSR280QA-TKS', 2, 43090, '4-струнная бас-гитара', 'bassguitars');
INSERT INTO public.instruments VALUES (19, 'IBANEZ RGD71ALMS-BAM', 1, 150870, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (20, 'IBANEZ GRX70QA-SB', 1, 32590, 'Электрогитара', 'electroguitars');
INSERT INTO public.instruments VALUES (21, 'Fernandes G4X(08)BLK', 2, 54990, '4-струнная бас-гитара', 'bassguitars');
INSERT INTO public.instruments VALUES (22, 'Solar Guitars AB2.5AN', 2, 149990, '5-струнная бас-гитара', 'bassguitars');
INSERT INTO public.instruments VALUES (23, 'TAMA CL52KRS-TPB', 5, 144470, 'Ударные', 'percussion');
INSERT INTO public.instruments VALUES (24, 'TAMA ST52H6C-SEM STAGESTAR', 5, 136590, 'Ударные', 'percussion');
INSERT INTO public.instruments VALUES (25, 'Pearl RS585C-C91', 5, 105990, 'Ударные', 'percussion');
INSERT INTO public.instruments VALUES (26, 'Rolling Rock JR-2232C Blue Snake', 5, 57990, 'Ударные', 'percussion');
INSERT INTO public.instruments VALUES (27, 'TAMA TW42RZS-WSBN STAR DRUM WALNUT SATIN BLACK', 5, 700000, 'Ударные', 'percussion');
INSERT INTO public.instruments VALUES (28, 'TAMA WBS52RZS-PBK STARCLASSIC WALNUT-BIRCH', 5, 500000, 'Ударные', 'percussion');
INSERT INTO public.instruments VALUES (29, 'TAMA TW42RZS-AIJB STAR DRUM WALNUT JAPAN', 5, 700000, 'Ударные', 'percussion');
INSERT INTO public.instruments VALUES (30, 'VOX VT20X', 3, 40990, 'Усилитель', 'amplifiers');
INSERT INTO public.instruments VALUES (31, 'VOX VX15-GT', 3, 29190, 'Усилитель', 'amplifiers');
INSERT INTO public.instruments VALUES (32, 'VOX CAMBRIDGE50', 3, 55690, 'Усилитель', 'amplifiers');
INSERT INTO public.instruments VALUES (33, 'E-WAVE GA-30', 3, 17390, 'Усилитель', 'amplifiers');
INSERT INTO public.instruments VALUES (34, 'KORG EK-50', 6, 56690, 'Клавишные', 'keyboards');
INSERT INTO public.instruments VALUES (35, 'YAMAHA PSR-E283', 6, 28490, 'Клавишные', 'keyboards');
INSERT INTO public.instruments VALUES (36, 'ROLAND TB-03', 6, 69300, 'Клавишные', 'keyboards');
INSERT INTO public.instruments VALUES (37, 'YAMAHA PSR-E383', 6, 36590, 'Клавишные', 'keyboards');
INSERT INTO public.instruments VALUES (38, 'YAMAHA PSR-A350', 6, 44470, 'Клавишные', 'keyboards');
INSERT INTO public.instruments VALUES (39, 'Ernie Ball 2148 Earthwood Phosphor Bronze', 4, 1280, 'Металлические струны', 'strings');
INSERT INTO public.instruments VALUES (40, 'Ernie Ball 2150 Earthwood Phosphor Bronze', 4, 1280, 'Металлические струны', 'strings');
INSERT INTO public.instruments VALUES (41, 'ROTOSOUND R10 STRINGS NICKEL REGULAR', 4, 990, 'Струны для электрогитары', 'strings');
INSERT INTO public.instruments VALUES (42, 'ROTOSOUND RH10 STRINGS NICKEL LIGHT TOP-HEAVY BOTTOM', 4, 990, 'Струны для электрогитары', 'strings');


--
-- TOC entry 4890 (class 0 OID 25674)
-- Dependencies: 224
-- Data for Name: purchases; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.purchases VALUES (2, 2, 2, '2024-12-28 07:26:40.861033');
INSERT INTO public.purchases VALUES (3, 3, 3, '2024-12-28 07:26:40.861033');
INSERT INTO public.purchases VALUES (4, 4, 4, '2024-12-28 07:26:40.861033');


--
-- TOC entry 4903 (class 0 OID 0)
-- Dependencies: 225
-- Name: administrators_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.administrators_id_seq', 2, true);


--
-- TOC entry 4904 (class 0 OID 0)
-- Dependencies: 221
-- Name: clients_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.clients_id_seq', 6, true);


--
-- TOC entry 4905 (class 0 OID 0)
-- Dependencies: 217
-- Name: instrument_types_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.instrument_types_id_seq', 7, true);


--
-- TOC entry 4906 (class 0 OID 0)
-- Dependencies: 219
-- Name: instruments_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.instruments_id_seq', 50, true);


--
-- TOC entry 4907 (class 0 OID 0)
-- Dependencies: 223
-- Name: purchases_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.purchases_id_seq', 6, true);


--
-- TOC entry 4732 (class 2606 OID 25697)
-- Name: admins administrators_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT administrators_pkey PRIMARY KEY (id);


--
-- TOC entry 4734 (class 2606 OID 25699)
-- Name: admins administrators_username_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT administrators_username_key UNIQUE (username);


--
-- TOC entry 4726 (class 2606 OID 25672)
-- Name: clients clients_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_email_key UNIQUE (email);


--
-- TOC entry 4728 (class 2606 OID 25670)
-- Name: clients clients_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_pkey PRIMARY KEY (id);


--
-- TOC entry 4722 (class 2606 OID 25649)
-- Name: instrument_types instrument_types_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instrument_types
    ADD CONSTRAINT instrument_types_pkey PRIMARY KEY (id);


--
-- TOC entry 4724 (class 2606 OID 25658)
-- Name: instruments instruments_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instruments
    ADD CONSTRAINT instruments_pkey PRIMARY KEY (id);


--
-- TOC entry 4730 (class 2606 OID 25680)
-- Name: purchases purchases_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_pkey PRIMARY KEY (id);


--
-- TOC entry 4735 (class 2606 OID 25659)
-- Name: instruments instruments_type_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instruments
    ADD CONSTRAINT instruments_type_id_fkey FOREIGN KEY (type_id) REFERENCES public.instrument_types(id) ON DELETE CASCADE;


--
-- TOC entry 4736 (class 2606 OID 25681)
-- Name: purchases purchases_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(id) ON DELETE CASCADE;


--
-- TOC entry 4737 (class 2606 OID 25686)
-- Name: purchases purchases_instrument_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.purchases
    ADD CONSTRAINT purchases_instrument_id_fkey FOREIGN KEY (instrument_id) REFERENCES public.instruments(id) ON DELETE CASCADE;


-- Completed on 2025-01-04 17:25:32

--
-- PostgreSQL database dump complete
--

