#pragma once
#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <memory>

#ifdef _DEBUG
#include <stdexcept>
#endif

/*
* Functionality regarding all tools.
*/
namespace Tools 
{
	// Models a single tool and stores arguments with its prior used values
	class Tool
	{
	private:
		//program arguments with list of prior used values, can be empty
		std::unordered_map<std::string, std::vector<std::string>> p_args;
	protected:
		Tool() {}

		//args: list of all possible/wished arguments
		Tool(const std::vector<std::string>& args) 
		{
			for (auto& arg : args) p_args[arg];
		}

		//set up channel to receive output of program
		virtual void setup_return_channel() = 0;
		//set up channel to send data to program
		virtual void setup_send_channel() = 0;
		//runs the program
		virtual void execute() = 0;
	public:
		/*
		* Runs the tool with the provided arguments args and data with size size.
		*/
		void* run(void* args, void* data, size_t size)
		{
			setup_return_channel();
			setup_send_channel();
			execute();
		}

		/*
		* Checks whether this tool is local or not.
		*/
		virtual bool is_local() = 0;

		//operator... duh
		virtual bool operator==(Tool& other) = 0;

		//adds the argument, returns true on success false if it already exists
		bool add_arg(const std::string& arg)
		{
			if (p_args.find(arg) != p_args.end()) return false;
			p_args[arg];
			return true;
		}

		//add arg value, returns true on success false if no such argument exists
		bool add_arg_val(const std::string& arg, const std::string& val)
		{
			if (p_args.find(arg) != p_args.end()) return false;
			p_args[arg].emplace_back(val);
			return true;
		}

		//add all arg values
		void add_arg_val(const std::string& arg, const std::list<std::string>& vals)
		{
			for (auto& val : vals) add_arg_val(arg, val);
		}

		//erases the arg completly, including its values
		void erase_arg(const std::string& arg)
		{
			auto it = p_args.find(arg);
			if (it == p_args.end()) return;
			p_args.erase(it);
		}

		//erases the arg's value
		void erase_arg_val(const std::string& arg, const std::string& val)
		{
			if (p_args.find(arg) == p_args.end()) return;
			std::vector<std::string>& list = p_args[arg];
			auto it = list.begin();
			while (it != list.end())
			{
				if (val.compare(*it) == 0) 
				{
					list.erase(it);
					break;
				}
			}
		}

		const auto& get_arg_map_entries() const { return p_args; }
	};

	// Models a tool on the local machine
	class Local_Tool : public virtual Tool
	{
	private:
		//path to program
		std::string p_path;
	protected:
		//set up channel to receive output of program
		void setup_return_channel() override 
		{
		
		}
		//set up channel to send data to program
		void setup_send_channel() override
		{
		
		}
		//runs the program
		void execute() override
		{
		
		}

	public:
		Local_Tool(const std::string& p, const std::vector<std::string>& args) : Tool(args), p_path(p) {}

		Local_Tool(const std::string& p) : p_path(p) {}

		/*
		* Checks whether this tool is local or not.
		*/
		bool is_local() override
		{
			return true;
		}

		//return immutable path to program
		const std::string& get_path() const { return p_path; }

		//set path to program
		void set_path(std::string& p) { p_path = p; }

		bool operator==(Tool& other) override 
		{
			Local_Tool* other_ptr = dynamic_cast<Local_Tool*>(&other);
			return other_ptr != nullptr && p_path.compare(other_ptr->p_path) == 0;
		}
	};

	// Registers tools with a short name
	class Tool_Registry 
	{
	private:
		std::unordered_map<std::string, std::shared_ptr<Tool>> p_map;
		Tool_Registry() {}
		~Tool_Registry() {}
	public:
		/*
		Tries to insert the tool under the given key in the registry.
		Will fail if the key is already in use.
		Returns true on success.
		*/
		bool insert(const std::string& key, const std::shared_ptr<Tool> ptr)
		{
			if (p_map.find(key) != p_map.end()) return false;
			p_map[key] = ptr;
			return true;
		}

		// get all entries in the registry
		const auto& get_entries() { return p_map; }

		// returns the single instance
		static Tool_Registry& get_instance()
		{
			static Tool_Registry s_instance;
			return s_instance;
		}

		//checks if the given short name is in the registry
		bool contains(const std::string& key)
		{
			return p_map.find(key) != p_map.end();
		}

		//returns the tool at the given key, same as operator[]
		std::shared_ptr<Tool> get_tool(const std::string& key) 
		{
#ifdef _DEBUG
			if (!contains(key)) throw std::invalid_argument("key is not registered"); //do check in debug mode
#endif
			return p_map[key];
		}

		//returns the tool at the given key, same as get_tool
		std::shared_ptr<Tool> operator[] (const std::string& key)
		{
			return get_tool(key);
		}

		//returns the key of the tool
		const std::string& get_key(std::shared_ptr<Tool> ptr)
		{
			for (const auto& [key, p] : p_map) 
			{
				if (p.get() == ptr.get()) return key;
			}
			throw std::invalid_argument("tool not registered");
		}

		Tool_Registry (const Tool_Registry&) = delete;
		void operator= (const Tool_Registry&) = delete;
	};

	// Contains all information for one step in the tool chain
	struct Tool_Chain_Entry
	{
		// tool to be used
		std::shared_ptr<Tool> tool;
		// arguments and values with which the tool should be launched
		std::unordered_map<std::string, std::string> launch_args;

		Tool_Chain_Entry(std::shared_ptr<Tool> tl) : tool(tl) {}
	};

	// complete tool chain
	struct Tool_Chain
	{
		//all steps in the tool chain
		std::vector<Tool_Chain_Entry> entries;
		//description
		std::string description;
	};

	// Registers all tool chains with their launch configurations
	class Tool_Chain_Registry
	{
	private:
		Tool_Chain_Registry() {}
		~Tool_Chain_Registry() {}
		std::vector<Tool_Chain> p_tool_chains;

	public:
		// returns the single instance
		static Tool_Chain_Registry& get_instance()
		{
			static Tool_Chain_Registry s_instance;
			return s_instance;
		}

		//inserts the new tool chain into the buffer
		void insert_tool_chain(const Tool_Chain& tc)
		{
			//update all used argument values
			for (auto& tc_entry : tc.entries)
				for (auto& [arg, val] : tc_entry.launch_args)
					tc_entry.tool->add_arg_val(arg, val);
			//store in registry
			p_tool_chains.push_back(tc);

			//resize buffers
			Tool_Chain& vec_tc = p_tool_chains[p_tool_chains.size() - 1];
			vec_tc.description.resize(256);
			for (auto it_entry = vec_tc.entries.begin(); it_entry != vec_tc.entries.end(); it_entry++)
			{
				for (auto it_map = it_entry->launch_args.begin(); it_map != it_entry->launch_args.end(); it_map++)
				{
					it_map->second.resize(256);
				}
			}
		}

		//get all entries
		auto& get_entries() 
		{
			return p_tool_chains;
		}

		Tool_Chain_Registry(const Tool_Chain_Registry&) = delete;
		void operator= (const Tool_Chain_Registry&) = delete;
	};
}
