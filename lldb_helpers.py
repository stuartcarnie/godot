import lldb
import lldb.formatters.Logger


class VectorViewSynthProvider:
    def __init__(self, valobj, dict):
        logger = lldb.formatters.Logger.Logger()
        self.valobj = valobj
        self.count = None
        self.data = None
        self.data_type = None
        self.data_size = None

    def num_children(self):
        return self.count

    def get_child_index(self, name):
        logger = lldb.formatters.Logger.Logger()
        try:
            return int(name.lstrip("[").rstrip("]"))
        except:
            return -1

    def get_child_at_index(self, index):
        logger = lldb.formatters.Logger.Logger()
        logger >> "Retrieving child " + str(index)
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        offset = index * self.data_size
        return self.data.CreateChildAtOffset(
            "[" + str(index) + "]", offset, self.data_type
        )

    def update(self):
        logger = lldb.formatters.Logger.Logger()
        # preemptively setting this to None - we might end up changing our mind
        # later
        self.count = None
        try:
            self.count = self.valobj.GetChildMemberWithName("_size").GetValueAsUnsigned(0)
            self.data = self.valobj.GetChildMemberWithName("_ptr")
            self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
            self.data_size = self.data_type.GetByteSize()
            return True
        except:
            self.count = 0
        return False

    def has_children(self):
        return self.count > 0


class LocalVectorSynthProvider:
    def __init__(self, valobj, dict):
        logger = lldb.formatters.Logger.Logger()
        self.valobj = valobj
        self.count = None
        self.data = None
        self.data_type = None
        self.data_size = None

    def num_children(self):
        return self.count

    def get_child_index(self, name):
        logger = lldb.formatters.Logger.Logger()
        try:
            return int(name.lstrip("[").rstrip("]"))
        except:
            return -1

    def get_child_at_index(self, index):
        logger = lldb.formatters.Logger.Logger()
        logger >> "Retrieving child " + str(index)
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        offset = index * self.data_size
        return self.data.CreateChildAtOffset(
            "[" + str(index) + "]", offset, self.data_type
        )

    def update(self):
        logger = lldb.formatters.Logger.Logger()
        # preemptively setting this to None - we might end up changing our mind
        # later
        self.count = None
        try:
            self.count = self.valobj.GetChildMemberWithName("count").GetValueAsUnsigned(0)
            self.data = self.valobj.GetChildMemberWithName("data")
            self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
            self.data_size = self.data_type.GetByteSize()
            return True
        except:
            self.count = 0
        return False

    def has_children(self):
        return self.count > 0


def __lldb_init_module(debugger: lldb.SBDebugger, dict):
    debugger.HandleCommand(
        'type synthetic add -l lldb_helpers.LocalVectorSynthProvider -x "LocalVector<" -x "TightLocalVector<" -w Godot'
    )
    debugger.HandleCommand(
        'type synthetic add -l lldb_helpers.VectorViewSynthProvider -x "VectorView<" -w Godot'
    )
