.PHONY: build test data solve clean-data demo eval clean

# build the C++ solver and tests
build:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build -j

# run the C++ unit tests
test: build
	ctest --test-dir build --output-on-failure

# generate a sample instance
data:
	python3 scripts/generate_instance.py --carers 8 --visits 60 --out data/town_60.txt

# solve the sample instance with the full method
solve: build data
	./build/careroute data/town_60.txt --method lns --out reports/town_60_solution.txt

# make messy CSVs and clean them (the data-engineering demo)
clean-data: data
	python3 scripts/make_messy_csv.py --instance data/town_60.txt --out data/raw
	python3 scripts/clean_data.py --carers data/raw/carers.csv --visits data/raw/visits.csv

# draw the routes map and the Gantt chart for the sample solution
demo: solve
	python3 scripts/visualise.py --instance data/town_60.txt --solution reports/town_60_solution.txt

# compare all methods across sizes and chart it
eval: build
	python3 scripts/run_eval.py

clean:
	rm -rf build reports/*.png reports/*.txt reports/*.json data/eval_* data/raw data/cleaned.txt
