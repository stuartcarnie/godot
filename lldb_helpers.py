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


class VectorSynthProvider:
    def __init__(self, valobj, dict):
        logger = lldb.formatters.Logger.Logger()
        self.valobj = valobj
        self.uint64_type = valobj.target.GetBasicType(lldb.eBasicTypeUnsignedLongLong)
        self.count = 0
        self.ref_count = None
        self.size = None
        self.data = None
        self.data_type = None
        self.data_size = None
        self.update()

    def num_children(self):
        return self.count + 2 if self.count > 0 else 0

    def get_child_index(self, name):
        if name == "size":
            return self.count + 0
        if name == "ref_count":
            return self.count + 1
        else:
            return int(name.lstrip("[").rstrip("]"))

    def get_child_at_index(self, index):
        if self.count == 0:
            return None
        elif index == self.count + 0:
            return self.size
        elif index == self.count + 1:
            return self.ref_count
        elif index < self.count:
            offset = index * self.data_size
            return self.data.CreateChildAtOffset(
                "[" + str(index) + "]", offset, self.data_type
            )
        else:
            return None

    def update(self):
        self.data = self.valobj.GetChildMemberWithName("_cowdata").GetChildMemberWithName("_ptr")
        self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_type.GetByteSize()

        if self.data.unsigned > 0:
            self.size = self.valobj.CreateValueFromAddress("size", self.data.unsigned - 8, self.uint64_type)
            self.ref_count = self.valobj.CreateValueFromAddress("ref_count", self.data.unsigned - 16, self.uint64_type)
            self.count = self.size.unsigned
        else:
            self.count = 0

    def has_children(self):
        return self.count > 0


def VectorSummaryProvider(valobj, dict):
    return "items=" + str(valobj.num_children - 2 if valobj.num_children > 0 else 0)


def __lldb_init_module(debugger: lldb.SBDebugger, dict):
    debugger.HandleCommand('type synthetic add -l lldb_helpers.LocalVectorSynthProvider -x "LocalVector<" -w Godot')
    debugger.HandleCommand('type synthetic add -l lldb_helpers.VectorViewSynthProvider -x "VectorView<" -w Godot')
    debugger.HandleCommand('type synthetic add -l lldb_helpers.VectorSynthProvider -x "Vector<.+>$" -w Godot')
    debugger.HandleCommand('type summary add "String" "CharString" --summary-string "${var._cowdata._ptr%s}" --category Godot')
    debugger.HandleCommand('type summary add "Vector2i" --summary-string "\{ w=${var.width}, h=${var.height} \}" --category Godot')
    debugger.HandleCommand('type summary add "Rect2i" --summary-string "\{ x=${var.position.x}, y=${var.position.height}, w=${var.size.width}, h=${var.size.height} \}" --category Godot')
    debugger.HandleCommand('type summary add -x "VectorView<" -x "LocalVector<" -x "TightLocalVector<" --expand --summary-string "${svar%#} items" --category Godot')
    debugger.HandleCommand('type summary add -F lldb_helpers.VectorSummaryProvider -x "Vector<.+>$" --expand --category Godot')
    debugger.HandleCommand('type category enable Godot')
